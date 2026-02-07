#include "stdafx.h"

#include "falclib.h"
#include "cpsurface.h"
#include "cpmanager.h"
#include "dispcfg.h"
#include "graphics/include/grinline.h"
#include "falclib/include/playerop.h"
#include "falclib/include/dispopts.h"
#include "graphics/include/renderow.h"

extern bool g_bCrackFix; //Wombat778 3-22-04 REMOVED 4-01-04
extern bool g_bFilter2DPit; //Wombat778 3-30-04

//====================================================//
// CPSurface::CPSurface
//====================================================//
CPSurface::CPSurface(SurfaceInitStr *psurfaceInitStr)
{

    mIdNum = psurfaceInitStr->idNum;

    mPersistant = psurfaceInitStr->persistant;

    mSrcRect.top = psurfaceInitStr->srcRect.top;
    mSrcRect.left = psurfaceInitStr->srcRect.left;
    mSrcRect.bottom = psurfaceInitStr->srcRect.bottom;
    mSrcRect.right = psurfaceInitStr->srcRect.right;

    mWidth = psurfaceInitStr->srcRect.right - psurfaceInitStr->srcRect.left;
    mHeight = psurfaceInitStr->srcRect.bottom - psurfaceInitStr->srcRect.top;

    mpOTWImage = psurfaceInitStr->pOtwImage;
    mpSourceBuffer = psurfaceInitStr->psrcBuffer;
    mpSurfaceBuffer = NULL;

    // OW
    m_pPalette = NULL;
}

//====================================================//
// CPSurface::~CPSurface
//====================================================//

CPSurface::~CPSurface()
{
    // OW
    for (int i = 0; i < (int)m_arrTex.size(); i++) delete m_arrTex[i];

    m_arrTex.clear();

    if (m_pPalette) delete m_pPalette;

    // nota bene: the manager creates the buffer for us,
    // but we must clean it up

    if (mpSurfaceBuffer)
    {
        mpSurfaceBuffer->Cleanup();
        delete mpSurfaceBuffer;
    }

    glReleaseMemory((char*) mpSourceBuffer);
}

//====================================================//
// CPSurface::CreateLit
//====================================================//

void CPSurface::CreateLit(void)
{
    if ( not DisplayOptions.bRender2DCockpit)
    {
        mpSurfaceBuffer = new ImageBuffer;

        // OW
        // mpSurfaceBuffer->Setup(&FalconDisplay.theDisplayDevice, mWidth, mHeight, SystemMem, None);

        MPRSurfaceType front = (FalconDisplay.theDisplayDevice.IsHardware() and DisplayOptions.bRender2DCockpit) ? LocalVideoMem : SystemMem;

        if ( not mpSurfaceBuffer->Setup(&FalconDisplay.theDisplayDevice, mWidth, mHeight, front, None) and front == LocalVideoMem)
        {
            // Retry with system memory if ouf video memory
#ifdef _DEBUG
            MonoPrint("CPSurface::CreateLit - Probably out of video memory. Retrying with system memory)\n");
#endif

            BOOL bResult = mpSurfaceBuffer->Setup(&FalconDisplay.theDisplayDevice, mWidth, mHeight, SystemMem, None);

            if ( not bResult) return;
        }

        mpSurfaceBuffer->SetChromaKey(0xFFFF0000);
    }

    else
    {
        try
        {
            const DWORD dwMaxTextureWidth = mpOTWImage->GetDisplayDevice()->GetDefaultRC()->m_pD3DHWDeviceDesc->dwMaxTextureWidth;
            const DWORD dwMaxTextureHeight = mpOTWImage->GetDisplayDevice()->GetDefaultRC()->m_pD3DHWDeviceDesc->dwMaxTextureHeight;

            m_pPalette = new PaletteHandle(mpOTWImage->GetDisplayDevice()->GetDefaultRC()->m_pDD, 32, 256);

            if ( not m_pPalette)
                throw _com_error(E_OUTOFMEMORY);

            // Check if we can use a single texture
            if ((int)dwMaxTextureWidth >= mWidth and (int)dwMaxTextureHeight >= mHeight)
            {
                TextureHandle *pTex = new TextureHandle;

                if ( not pTex)
                    throw _com_error(E_OUTOFMEMORY);

                m_pPalette->AttachToTexture(pTex);

                if ( not pTex->Create("CPSurface", MPR_TI_PALETTE bitor MPR_TI_CHROMAKEY, 8, mWidth, mHeight))
                    throw _com_error(E_FAIL);

                if ( not pTex->Load(0, 0xFFFF0000, (BYTE*) mpSourceBuffer, true, true)) // soon to be re-loaded by CPSurface::Translate3D
                    throw _com_error(E_FAIL);

                m_arrTex.push_back(pTex);
            }

            else
            {
                // Create tiles
                int nRows = 0; // JB 010220 CTD

                if (dwMaxTextureHeight) // JB 010220 CTD
                {
                    nRows = mHeight / dwMaxTextureHeight; // JB 010220 CTD

                    if (mHeight % dwMaxTextureHeight) nRows++; // JB 010404 CTD enclosed in brackets
                }

                int nColumns = 0; // JB 010220 CTD

                if (dwMaxTextureWidth) // JB 010220 CTD
                {
                    nColumns = mWidth / dwMaxTextureWidth;

                    if (mWidth % dwMaxTextureWidth) nColumns++; // JB 010404 CTD enclosed in brackets
                }

                DWORD dwHeightRemaining = mHeight;

                for (int y = 0; y < nRows; y++)
                {
                    DWORD dwWidthRemaining = mWidth;

                    for (int x = 0; x < nColumns; x++)
                    {
                        TextureHandle *pTex = new TextureHandle;

                        if ( not pTex)
                            throw _com_error(E_OUTOFMEMORY);

                        m_pPalette->AttachToTexture(pTex);

                        if ( not pTex->Create("CPSurface - Tile", MPR_TI_PALETTE bitor MPR_TI_CHROMAKEY, 8,
                                          (UInt16)min(dwMaxTextureWidth, dwWidthRemaining), (UInt16)min(dwMaxTextureHeight, dwHeightRemaining)))
                            throw _com_error(E_FAIL);

                        DWORD dwOffset = (y * dwMaxTextureHeight) * mWidth;
                        dwOffset += x * dwMaxTextureWidth;

                        if ( not pTex->Load(0, 0xFFFF0000, (BYTE*) mpSourceBuffer + dwOffset, true, true, mWidth)) // soon to be re-loaded by CPSurface::Translate3D
                            throw _com_error(E_FAIL);

                        m_arrTex.push_back(pTex);
                        dwWidthRemaining -= dwMaxTextureWidth;
                    }

                    dwHeightRemaining -= dwMaxTextureHeight;
                }
            }
        }

        catch (_com_error e)
        {
            MonoPrint("CPSurface::CreateLit - Error 0x%X (%s)\n", e.Error(), e.ErrorMessage());
            DiscardLit();
        }
    }
}

//====================================================//
// CPSurface::DiscardLit
//====================================================//

void CPSurface::DiscardLit(void)
{
    if (mpSurfaceBuffer)
        mpSurfaceBuffer->Cleanup();

    delete mpSurfaceBuffer;
    mpSurfaceBuffer = NULL;

    for (int i = 0; i < (int)m_arrTex.size(); i++) delete m_arrTex[i];

    m_arrTex.clear();

    if (m_pPalette)
    {
        delete m_pPalette; // JPO - memory leak fix
        m_pPalette = NULL;
    }
}



//====================================================//
// CPSurface::Display
//====================================================//

void CPSurface::DisplayBlit(BYTE blitType, BOOL Persistance, RECT *pDestRect, int xPanelOffset, int yPanelOffset)
{
    if (m_arrTex.size())
        return; // handled in DisplayBlit3D

    RECT blitRect;
    RECT destRect;

    destRect.top = pDestRect->top;
    destRect.left = pDestRect->left;
    destRect.bottom = pDestRect->bottom;
    destRect.right = pDestRect->right;

    blitRect.top = 0;
    blitRect.left = 0;
    blitRect.bottom = mHeight;
    blitRect.right = mWidth;

    destRect.top = (LONG)(OTWDriver.pCockpitManager->mVScale * (destRect.top + yPanelOffset));
    destRect.left = (LONG)(OTWDriver.pCockpitManager->mHScale * (destRect.left + xPanelOffset));
    destRect.bottom = (LONG)(OTWDriver.pCockpitManager->mVScale * (destRect.bottom + yPanelOffset + 1));
    destRect.right = (LONG)(OTWDriver.pCockpitManager->mHScale * (destRect.right + xPanelOffset + 1));

    if (Persistance == NONPERSISTANT)
    {
        if (blitType == TRANSPARENT)
            mpOTWImage->ComposeTransparent(mpSurfaceBuffer, &blitRect, &destRect);

        else
            mpOTWImage->Compose(mpSurfaceBuffer, &blitRect, &destRect);
    }

    else
    {
        /* do it to both buffers */
    }
}

void CPSurface::DisplayBlit3D(BYTE blitType, BOOL Persistance, RECT *pDestRect, int xPanelOffset, int yPanelOffset)
{
#ifdef FF_LINUX
    {
        static int db3dDiag = 0;
        if (db3dDiag < 30) {
            db3dDiag++;
            fprintf(stderr, "[CPSurf::DB3D] #%d m_arrTex.size=%d blitType=%d Persist=%d rect=(%ld,%ld,%ld,%ld)\n",
                    db3dDiag, (int)m_arrTex.size(), (int)blitType, (int)Persistance,
                    pDestRect->left, pDestRect->top, pDestRect->right, pDestRect->bottom);
            fflush(stderr);
        }
    }
#endif
    if ( not m_arrTex.size())
        return; // handled in DisplayBlit

    RECT destRect;

    destRect.top = pDestRect->top;
    destRect.left = pDestRect->left;
    destRect.bottom = pDestRect->bottom;
    destRect.right = pDestRect->right;

    destRect.top = (LONG)(OTWDriver.pCockpitManager->mVScale * (destRect.top + yPanelOffset));
    destRect.left = (LONG)(OTWDriver.pCockpitManager->mHScale * (destRect.left + xPanelOffset));
    destRect.bottom = (LONG)(OTWDriver.pCockpitManager->mVScale * (destRect.bottom + yPanelOffset + 1));
    destRect.right = (LONG)(OTWDriver.pCockpitManager->mHScale * (destRect.right + xPanelOffset + 1));

    //Wombat778 3-22-04 Improves "cracked" cockpit. By expanding the target rectangles by 0.5 on each side, the directx texel alignment issue is reduced.
    // COBRA - RED - Wombat... Do u know what a Long Type is...?
    /* if (g_bCrackFix)
     {
     destRect.top -= 0.5f;
     destRect.left -= 0.5f;
     destRect.bottom += 0.5f;
     destRect.right += 0.5f;
     }
    */

    if (Persistance == NONPERSISTANT)
    {
        if (m_arrTex.size() == 1)
        {
            // One pass
            TextureHandle *pTex = m_arrTex[0];

            // Setup vertices
            float fStartU = 0;
            float fMaxU = (float) pTex->m_nWidth / (float) pTex->m_nActualWidth;
            fMaxU -= fStartU;

            float fStartV = 0;
            float fMaxV = (float) pTex->m_nHeight / (float) pTex->m_nActualHeight;
            fMaxV -= fStartV;

            TwoDVertex pVtx[4];
            ZeroMemory(pVtx, sizeof(pVtx));
            pVtx[0].x = (float)destRect.left;
            pVtx[0].y = (float)destRect.top;
            pVtx[0].u = fStartU;
            pVtx[0].v = fStartV;
            pVtx[0].r = pVtx[0].g = pVtx[0].b = pVtx[0].a = 1.0f;
            pVtx[1].x = (float)destRect.right;
            pVtx[1].y = (float)destRect.top;
            pVtx[1].u = fMaxU;
            pVtx[1].v = fStartV;
            pVtx[1].r = pVtx[1].g = pVtx[1].b = pVtx[1].a = 1.0f;
            pVtx[2].x = (float)destRect.right;
            pVtx[2].y = (float)destRect.bottom;
            pVtx[2].u = fMaxU;
            pVtx[2].v = fMaxV;
            pVtx[2].r = pVtx[2].g = pVtx[2].b = pVtx[2].a = 1.0f;
            pVtx[3].x = (float)destRect.left;
            pVtx[3].y = (float)destRect.bottom;
            pVtx[3].u = fStartU;
            pVtx[3].v = fMaxV;
            pVtx[3].r = pVtx[3].g = pVtx[3].b = pVtx[3].a = 1.0f;

            // COBRA - RED - Pit Vibrations
            OTWDriver.pCockpitManager->AddTurbulence(pVtx);

            // Setup state
            if (blitType == TRANSPARENT)
            {
                if (g_bFilter2DPit) //Wombat778 3-30-04 Added option to filter
                    OTWDriver.renderer->context.RestoreState(STATE_CHROMA_TEXTURE);
                else
                    OTWDriver.renderer->context.RestoreState(STATE_ALPHA_TEXTURE_NOFILTER);
            }

            else
            {
                if (g_bFilter2DPit) //Wombat778 3-30-04 Added option to filter
                    OTWDriver.renderer->context.RestoreState(STATE_TEXTURE);
                else
                    OTWDriver.renderer->context.RestoreState(STATE_TEXTURE_NOFILTER);
            }

            OTWDriver.renderer->context.SelectTexture1((intptr_t) pTex);

            // Render it (finally)
            OTWDriver.renderer->context.DrawPrimitive(MPR_PRM_TRIFAN, MPR_VI_COLOR bitor MPR_VI_TEXTURE, 4, pVtx, sizeof(pVtx[0]));
#ifdef FF_LINUX
            {
                static int drawDiag = 0;
                if (drawDiag < 15) {
                    drawDiag++;
                    fprintf(stderr, "[CPSurf::DB3D.Draw] #%d 1-pass tex=%p %dx%d dest=(%ld,%ld)-(%ld,%ld) vtx0=(%.1f,%.1f) a=%.2f\n",
                            drawDiag, (void*)pTex, pTex->m_nWidth, pTex->m_nHeight,
                            destRect.left, destRect.top, destRect.right, destRect.bottom,
                            pVtx[0].x, pVtx[0].y, pVtx[0].a);
                    fflush(stderr);
                }
            }
#endif
        }

        else
        {
            // Tile

            // Setup state
            if (blitType == TRANSPARENT)
            {
                if (g_bFilter2DPit) //Wombat778 3-30-04 Added option to filter
                    OTWDriver.renderer->context.RestoreState(STATE_CHROMA_TEXTURE);
                else
                    OTWDriver.renderer->context.RestoreState(STATE_ALPHA_TEXTURE_NOFILTER);
            }
            else
            {
                if (g_bFilter2DPit) //Wombat778 3-30-04 Added option to filter
                    OTWDriver.renderer->context.RestoreState(STATE_TEXTURE);
                else
                    OTWDriver.renderer->context.RestoreState(STATE_TEXTURE_NOFILTER);
            }

            const DWORD dwMaxTextureWidth = mpOTWImage->GetDisplayDevice()->GetDefaultRC()->m_pD3DHWDeviceDesc->dwMaxTextureWidth;
            const DWORD dwMaxTextureHeight = mpOTWImage->GetDisplayDevice()->GetDefaultRC()->m_pD3DHWDeviceDesc->dwMaxTextureHeight;

            int nRows = mHeight / dwMaxTextureHeight;

            if (mHeight % dwMaxTextureHeight) nRows++;

            int nColumns = mWidth / dwMaxTextureWidth;

            if (mWidth % dwMaxTextureWidth) nColumns++;

            TwoDVertex pVtx[4];
            ZeroMemory(pVtx, sizeof(pVtx));
            TextureHandle *pTex;
            int left, right, top, bottom;

            DWORD dwHeightRemaining = mHeight;
            top = destRect.top;

            for (int y = 0; y < nRows; y++)
            {
                DWORD dwWidthRemaining = mWidth;
                left = destRect.left;
                bottom = top + min(dwMaxTextureHeight, dwHeightRemaining);

                for (int x = 0; x < nColumns; x++)
                {
                    pTex = m_arrTex[y * nColumns + x];
                    right = left + min(dwMaxTextureWidth, dwWidthRemaining);

                    // Setup vertices
                    float fStartU = 0;
                    float fMaxU = (float) pTex->m_nWidth / (float) pTex->m_nActualWidth;
                    fMaxU -= fStartU;

                    float fStartV = 0;
                    float fMaxV = (float) pTex->m_nHeight / (float) pTex->m_nActualHeight;
                    fMaxV -= fStartV;

                    pVtx[0].x = (float)left;
                    pVtx[0].y = (float)top;
                    pVtx[0].u = fStartU;
                    pVtx[0].v = fStartV;
                    pVtx[0].r = pVtx[0].g = pVtx[0].b = pVtx[0].a = 1.0f;
                    pVtx[1].x = (float)right;
                    pVtx[1].y = (float)top;
                    pVtx[1].u = fMaxU;
                    pVtx[1].v = fStartV;
                    pVtx[1].r = pVtx[1].g = pVtx[1].b = pVtx[1].a = 1.0f;
                    pVtx[2].x = (float)right;
                    pVtx[2].y = (float)bottom;
                    pVtx[2].u = fMaxU;
                    pVtx[2].v = fMaxV;
                    pVtx[2].r = pVtx[2].g = pVtx[2].b = pVtx[2].a = 1.0f;
                    pVtx[3].x = (float)left;
                    pVtx[3].y = (float)bottom;
                    pVtx[3].u = fStartU;
                    pVtx[3].v = fMaxV;
                    pVtx[3].r = pVtx[3].g = pVtx[3].b = pVtx[3].a = 1.0f;

                    // COBRA - RED - Pit Vibrations
                    OTWDriver.pCockpitManager->AddTurbulence(pVtx);

                    OTWDriver.renderer->context.SelectTexture1((intptr_t) pTex);

                    // Render it (finally)
                    OTWDriver.renderer->context.DrawPrimitive(MPR_PRM_TRIFAN, MPR_VI_COLOR bitor MPR_VI_TEXTURE, 4, pVtx, sizeof(pVtx[0]));

                    dwWidthRemaining -= dwMaxTextureWidth;
                    left += dwMaxTextureWidth;
                }

                dwHeightRemaining -= dwMaxTextureHeight;
                top += dwMaxTextureHeight;
            }
        }
    }

    else
    {
        /* do it to both buffers */
    }
}


void CPSurface::Translate(WORD* palette16)
{
    if (mpSurfaceBuffer)
        Translate8to16(palette16, mpSourceBuffer, mpSurfaceBuffer); // 8 bit color indexes of individual surfaces
} // 16 bit ImageBuffers

// OW
void CPSurface::Translate(DWORD* palette32)
{
    if (mpSurfaceBuffer)
        Translate8to32(palette32, mpSourceBuffer, mpSurfaceBuffer); // 8 bit color indexes of individual surfaces
}

void CPSurface::Translate3D(DWORD* palette32)
{
#ifdef FF_LINUX
    {
        static int t3dDiag = 0;
        if (t3dDiag < 5) {
            t3dDiag++;
            fprintf(stderr, "[Translate3D] #%d m_pPalette=%p m_arrTex.size()=%d\n",
                    t3dDiag, (void*)m_pPalette, (int)m_arrTex.size());
            if (m_pPalette) {
                fprintf(stderr, "[Translate3D] #%d palette nAttached=%d\n",
                        t3dDiag, (int)m_pPalette->m_arrAttachedTextures.size());
            }
            if (!m_arrTex.empty()) {
                TextureHandle *pTex = m_arrTex[0];
                fprintf(stderr, "[Translate3D] #%d tex[0]=%p m_pDDS=%p m_pImageData=%p m_eSurfFmt=%d\n",
                        t3dDiag, (void*)pTex, (void*)pTex->m_pDDS, (void*)pTex->m_pImageData, (int)pTex->m_eSurfFmt);
            }
            fflush(stderr);
        }
    }
#endif
    if (m_pPalette)
        m_pPalette->Load(MPR_TI_PALETTE, 32, 0, 256, (BYTE*) palette32);
#ifdef FF_LINUX
    {
        static int t3dPostDiag = 0;
        if (t3dPostDiag < 3 && !m_arrTex.empty()) {
            t3dPostDiag++;
            TextureHandle *pTex = m_arrTex[0];
            // Check texture data via Lock/Unlock
            if (pTex->m_pDDS) {
                DDSURFACEDESC2 ddsd;
                memset(&ddsd, 0, sizeof(ddsd));
                ddsd.dwSize = sizeof(ddsd);
                HRESULT hr = pTex->m_pDDS->Lock(NULL, &ddsd, DDLOCK_READONLY, NULL);
                if (SUCCEEDED(hr)) {
                    int nonZero = 0;
                    int checkLen = ddsd.lPitch * ddsd.dwHeight;
                    if (checkLen > 4096) checkLen = 4096;
                    BYTE* data = (BYTE*)ddsd.lpSurface;
                    for (int b = 0; b < checkLen; b++) {
                        if (data[b] != 0) nonZero++;
                    }
                    fprintf(stderr, "[Translate3D.Post] #%d %dx%d pitch=%d nonZero4K=%d bpp=%d\n",
                            t3dPostDiag, (int)ddsd.dwWidth, (int)ddsd.dwHeight, (int)ddsd.lPitch,
                            nonZero, (int)ddsd.ddpfPixelFormat.dwRGBBitCount);
                    pTex->m_pDDS->Unlock(NULL);
                } else {
                    fprintf(stderr, "[Translate3D.Post] #%d Lock FAILED hr=0x%x\n", t3dPostDiag, (unsigned)hr);
                }
            }
            fflush(stderr);
        }
    }
#endif
}
