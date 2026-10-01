# FreeFalcon 6 — Pilot's Manual (Linux port)

*New manual, 2026-10-01, for the native Linux build of FreeFalcon 6 (FFViper) on the Falcon 4.0 engine.*

**Where this comes from**

- **Game procedures:** *Falcon 4.0: Allied Force* manual (`~/sgl/SAT/DOC/F4AFManual.pdf`). It is well
  written and mostly applies to FreeFalcon, but its screens and many of its keys differ.
- **FreeFalcon additions:** the FreeFalcon companion (`ff6_manual.pdf`, mostly carried over from FF 5.0)
  for the features FreeFalcon added, chiefly naval operations and multiplayer ports.
- **Linux specifics:** the Linux port's own notes.
- **Keys:** every key comes from the program's own data ([section 15](#15-keyboard-reference)).

Where a document and the program disagree, the program wins; [section 16](#16-corrections-to-the-older-manuals)
lists the corrections.

For the depth of the F-16 avionics (radar sub-modes, every MFD page, the ICP/DED, each console
switch), the Allied Force manual (chapters 17–21) remains the reference; this manual covers what you
need to fly and fight, plus every place FreeFalcon on Linux differs.

---

## Contents

1. [What FreeFalcon is](#1-what-freefalcon-is)
2. [Installing and starting](#2-installing-and-starting)
3. [The main menu](#3-the-main-menu)
4. [Logbook and Setup](#4-logbook-and-setup)
5. [Instant Action](#5-instant-action)
6. [Dogfight](#6-dogfight)
7. [Tactical Engagement](#7-tactical-engagement)
8. [Campaign](#8-campaign)
9. [Flying the F-16](#9-flying-the-f-16)
10. [Sensors and weapons](#10-sensors-and-weapons)
11. [Views](#11-views)
12. [Radio](#12-radio)
13. [Carrier operations](#13-carrier-operations)
14. [Multiplayer](#14-multiplayer)
15. [Keyboard reference](#15-keyboard-reference)
16. [Corrections to the older manuals](#16-corrections-to-the-older-manuals)
17. [ACMI and screenshots](#17-acmi-and-screenshots)
18. [Troubleshooting](#18-troubleshooting)

---

## 1. What FreeFalcon is

FreeFalcon is the community's continuation of MicroProse's *Falcon 4.0*. It is a study simulation of
the F-16C Block 50/52 inside a **dynamic campaign**: the war is fought by thousands of air and ground
units whether you fly or not, and your sorties are part of it. FreeFalcon 6 adds:
- many flyable aircraft besides the Viper, with 3-D cockpits;
- naval aviation from working carriers;
- several theatres;
- revised training and tactical engagements.

The F-16 remains the most completely modelled aircraft.

The game has five ways to play: **Instant Action** (fly and fight now), **Dogfight**, **Tactical
Engagement** (34 training missions plus scenario missions and an editor), **Campaign**, and
**multiplayer** across Dogfight, TE and Campaign.

---

## 2. Installing and starting

**The AppImage**

```bash
chmod +x FreeFalcon-x86_64-<ver>.AppImage
./FreeFalcon-x86_64-<ver>.AppImage
```

The first run copies the game data, about **14 GB**, to `~/.local/share/freefalcon/FreeFalcon6`
(`FF_HOME=<dir>` changes the parent folder). Later runs start at once. To use an existing install
and skip the copy, set `FF_GAMEDATA=/path/to/FreeFalcon6`. The AppImage starts the game windowed.

**The binary directly** (developer layout)

```bash
FFViper -d /path/to/FreeFalcon6 [-w | -f] [-nosound] [-port <n>]
```

| Option | Effect |
|---|---|
| `-d` | Data folder. `FF_DATA_DIR` does the same. |
| `-w` / `-f` | Windowed / full screen |
| `-nosound` | No audio |
| `-port <n>` | Force the local UDP port used for multiplayer |

**Keys the Linux build handles itself.** These are processed before the game's key file:

| Key | Effect |
|---|---|
| **F11** | Full screen on or off |
| **F5** (menus only) | Quick-launch Instant Action |
| **Shift+Num8/4/6/2** (in flight) | Select the front/left/right/down 2-D cockpit panel. The key also reaches the sim as a DMS command; see [section 18](#18-troubleshooting). |
| **Esc** | Never quits the program. In flight it opens the exit menu ([section 9](#9-flying-the-f-16)). |

Quit from the main menu's **Exit**, or close the window.

---

## 3. The main menu

| Item | What it does |
|---|---|
| **Instant Action** | Immediate air-to-air or air-to-ground fight ([section 5](#5-instant-action)) |
| **Dogfight** | Furball, Team Furball or Match Play against AI and/or people ([section 6](#6-dogfight)) |
| **Tactical Engagement** | Training missions, scenario missions, and the mission builder ([section 7](#7-tactical-engagement)) |
| **Campaign** | The dynamic war ([section 8](#8-campaign)) |
| **Logbook** | Pilots, callsigns, records, medals; each pilot has their own settings and key file |
| **ACMI** | Replay flight recordings ([section 17](#17-acmi-and-screenshots)) |
| **Tactical Reference** | Aircraft, vehicles, weapons, and threat radar sounds |
| **Setup** | Simulation, Graphics, Sound and Controllers ([section 4](#4-logbook-and-setup)) |
| **Comms** | Connect to, or host, a multiplayer session ([section 14](#14-multiplayer)) |
| **Theatre** | Choose the theatre. TE and Campaign lists belong to the current theatre, and the choice is remembered. |

Useful conventions in the menus:
- Right-click on maps and lists for context menus.
- Most list rows respond to a click on their **text** rather than the empty part of the row.
- **Commit** moves forward and **Back** returns.

---

## 4. Logbook and Setup

**Logbook.** Create a pilot and give them a **callsign**; in multiplayer the two pilots need different
callsigns. The Logbook keeps kills, missions, the Ace Factor and medals. Settings and the key file are
stored **per pilot** (the default pilot file is `config/Viper.pop`).

**Setup → Simulation**

| Setting | Choices and effect |
|---|---|
| **Skill level** | Recruit, Cadet, Rookie, Veteran or Ace; sets the items below as a group |
| **Realism rating** | 0–100, worked out from the settings. It multiplies Instant Action scores and feeds the Ace Factor. |
| **Flight model** | *Accurate* (deep stalls, flat spins) or *Simplified* (more forgiving, easier landings) |
| **Avionics** | *Easy* (radar sees everything in range), *Simplified* (forward only), or *Realistic* (the APG-68 and its sub-modes) |
| **Weapon effects** | Exaggerated, Enhanced or Accurate (the blast radius and damage you need to achieve) |
| **Autopilot** | *3-axis* holds attitude. *Steerpoint* flies the route. *Combat* fights for you and refuels itself. |
| **Air refuelling** | Realistic, Simplified or Easy (how forgiving the boom is) |
| **Padlocking** | *Realistic* (60° view, within visual range), *Enhanced* (any direction), or Disabled |
| **Toggles** | Invulnerability, unlimited fuel, unlimited chaff and flares, no collisions, no blackout, labels, disable clouds, bullseye radio calls, ACMI file size (about 100 KB per minute) |

**Setup → Graphics**
- Resolution, landscape texture and detail distance, object detail, special effects, and texture
  quality.
- **Canopy cues:** lift line, reflection, both, or none.
- Clouds, and a preview window.

On Linux the image is drawn through OpenGL; the "video driver/card" lists are informational.

**Setup → Sound.** Volumes for engine, Sidewinder tone, RWR, cockpit (VMS) and radio, plus a master
volume. Each has a test button.

**Setup → Controllers**
- Choose the joystick, throttle and rudder, and centre them.
- The **key mapping** list shows every command. Click a key and press a new one to rebind it, then
  **Save** to a new key file, which is then tied to the current pilot. A key can drive only one
  command.

The default key file is `config/keystrokes.key`. Two others ship with the game: `laptop.key` and
`CougarPitbulders.key`.

---

## 5. Instant Action

Endless enemies until you quit, land, eject or die.

| Setting | Choices |
|---|---|
| **Mission** | *Fighter Sweep* (air-to-air) or *Moving Mud* (ground attack); sets your loadout |
| **Wave** | Recruit to Ace; harder settings bring more fighters and fewer transports |
| **Air defences** | SAMs and AAA |
| **Weather** | Fog height and distance |
| **Map** | Drag the grey square to choose where you start, and set the clock for day or night |

**Fly** starts the fight.

**Scoring.** Kills count, multiplied by your realism rating; bombs and missiles that miss cost
points (guns are free). Good scores reach the **Sierra Hotel** board.

In the menus, **F5** is a Linux-build shortcut straight into Instant Action.

---

## 6. Dogfight

Up to four teams, people and AI in any mix.

| Type | Rules |
|---|---|
| **Furball** | Everyone for themselves. +1 per kill, −1 for crashing or ejecting undamaged. You respawn with fresh weapons until someone reaches the points limit. |
| **Team Furball** | The same, scored by team. Killing a teammate costs your team a point. |
| **Match Play** | Rounds with identical starts. A team scores when it kills every opponent. No joining once it has begun, and the dead sit out the round. |

**Setup**
- **Add aircraft** (or right-click in a team's quadrant): type, skill and markings.
- **Join** a team (or right-click → Join).
- **Options:** rear-aspect IR, all-aspect IR and radar missile counts, fog, unlimited guns (otherwise
  510 rounds), ECM pods, start range (5–60 nm), start altitude (2,000–60,000 ft), and points to win.
- **Save** keeps a setup for later.

**F** starts the flight recorder. Review the fight afterwards in ACMI.

---

## 7. Tactical Engagement

Tactical Engagement (TE) is the training school, the scenario library and the mission editor. Its
lists belong to the **current theatre** (Korea for the training missions).

**Playing a TE**
1. **Tactical Engagement**, then pick the **Training** or **Missions** list (Saved/Online tabs hold
   your own and network games).
2. Click a mission's name.
3. On the theatre map, click the **team** you want to fly for, then **Commit**.
4. The **Mission Schedule** lists the flights. Pick one, read its **Flight Plan** and **Briefing**,
   adjust **Munitions** if you like, then press **Fly**.
5. Pick a flight that is still in Briefing. You can join a flight already airborne up to its IP, but
   stop the clock while you read, or your flight leaves without you.

**The 34 training missions** (Korea) build from basic handling to weapons employment. Every one was
loaded and flown into the cockpit on this build on 2026-10-01; see the table at the end of this
section.

**Scenario missions.** Included TEs: "Sink the Kuz", "Fly the BlackBird", "Havin' Fun Strike", "The
Pits – Blue Aircraft", and naval templates ("Naval Ops – Modern / ODS / Vietnam", "Naval East / Yellow
Sea", "Rogue Navy"). The naval templates place carrier groups with most of the flyable naval aircraft;
they are the place to practise carrier operations before a campaign.

> **A TE's own data can cancel flights.** In *Havin' Fun Strike* the US and South Korea are set as
> separate teams at war, with the bases South Korean, so the game cancels the US flights as it loads.
> The flight list then shows almost nothing to fly. That is the scenario's data, not a fault. *Sink
> the Kuz* is a reliable choice.

**Building a mission (Mission Builder)**
1. **Tactical Engagement → Mission Builder** (a new engagement starts with you on the US/blue team).
2. Right-click the map to show targets, for example *Installations*, and zoom with **+**.
3. **Add Package**, then click the target.
4. Lock the take-off time (padlock icon green), then **New** to add a flight: type F-16C-52, role
   Strike.
5. **OK.** The planner draws the route.
6. **Flight Plan**: step to the TGT steerpoint, **Assign** the specific target (Recon shows a photo).
   The **Briefing** fills itself in.
7. To add opposition, switch teams with the **Team Selector**, advance the clock until your flight is
   en route, and **Add Flight** an intercept against it.
8. Set **Teams** and **Victory Conditions** (points per objective), then save. Saved engagements
   appear in the Saved list.

When you only *play* a TE, you can plan packages and give ground orders, but you cannot add units or
edit teams and victory conditions.

**The training missions** (Korea theatre; all 34 load and reach the cockpit on this build, 2026-10-01):

| # | Mission | # | Mission |
|---|---|---|---|
| 01 | Basic Handling | 18 | A-G Radar Modes |
| 02 | Takeoff | 19 | Bombs with CCRP |
| 03 | Max Turn at Corner | 20 | Bombs with CCIP |
| 04 | Max Turn Above Corner | 21 | Bombs With Dive-Toss |
| 05 | Max Turn Below Corner | 22 | 20mm Cannon (A-G) |
| 06 | Min Altitude Split S | 23 | Rockets |
| 07 | High-Speed Over Top | 24 | Mavericks |
| 08 | Low-Speed Over Top | 25 | Laser-Guided Bombs |
| 09 | Landing Final Approach | 26 | HARMs |
| 10 | Instrument Landing | 27 | Refueling |
| 11 | Flameout Landing | 28 | Missile Threat |
| 12 | Nav and Timing | 29 | Offensive BFM |
| 13 | A-A Radar Modes | 30 | Defensive BFM |
| 14 | 20mm Cannon (A-A) | 31 | Head-on BFM |
| 15 | AIM-9 Sidewinder | 32 | JDAMs |
| 16 | AIM-120 AMRAAM | 33 | F-18 Carrier Takeoff |
| 17 | AIM-7 Sparrow | 34 | F-18 Carrier Landing |

---

## 8. Campaign

A dynamic war. Campaigns in the Korea theatre:

| Campaign | Difficulty | Situation |
|---|---|---|
| **Tiger Spirit** | Easy | The North attacks; a defensive start |
| **Rolling Fire** | Medium | The war is under way |
| **Iron Fortress** | Advanced | The hardest balance of forces |

**Flow**
1. **Campaign** → choose a campaign and a **squadron**, then **Commit**.
2. The **planning screen** shows the map, the **mission schedule** (ATO) for your squadron, the
   package, and the clock.
3. Pick a flight, then check its **Flight Plan**, **Briefing** and **Munitions**. You can load
   your wingmen too.
4. **Fly.** Choose *Ramp* (cold start), *Taxi* (available up to two minutes before departure) or
   *Takeoff* (on the runway at your take-off time).
5. After the flight, the **debrief**. The war keeps going whether or not you fly.

**Priorities.** You can set target priorities (air superiority, interdiction, close air support,
and so on), or let HQ decide.

**Time.** In the planning screen the clock can be paused, run normally, or accelerated. In
multiplayer the clock runs at the slowest speed any player asks for.

**Saving.** Campaigns save and load from the Campaign screen.

---

## 9. Flying the F-16

**Getting airborne quickly** (Takeoff start, engine running):
1. Release the parking brake if it is set (**Alt+P**).
2. Advance the throttle: **=** forward, **-** back, **Alt+-** idle. **Ctrl+=** is full afterburner
   and **Ctrl+-** minimum afterburner.
3. Steer with the rudder, **,** (left) and **.** (right). **Shift+/** engages nosewheel steering.
4. Rotate at about 150 kt.
5. Raise the gear (**G**) before 300 kt.

**Basic controls**

| Control | Keys |
|---|---|
| Pitch / roll | Joystick, or the arrow keys |
| Rudder | **,** / **.** |
| Throttle | **=** / **-**, **Alt+-** idle, **Ctrl+=** full afterburner, **Ctrl+-** minimum afterburner |
| Speed brakes | **Shift+B** open, **Ctrl+B** close |
| Landing gear | **G** |
| Wheel brakes | **K** |
| Parking brake | **Alt+P** |
| Hook | **Ctrl+K** |
| Canopy | **Ctrl+Shift+C** |
| Eject | **Ctrl+E** |
| Autopilot | **A** (the kind of autopilot is set in Setup) |
| AP disconnect | **Ctrl+4** |
| Manual trim | **Alt+Shift+Home** nose up, **Alt+Shift+End** nose down, **Alt+Shift+Delete** roll left |
| Pause / freeze | **P** pause, **Shift+P** freeze (physics stop, avionics live; good for study) |
| Time | **Tab** 2× time compression, **CapsLock** 4×, **Shift+Tab** / **Shift+CapsLock** step it up or down |
| Steerpoints | **S** next, **Shift+S** previous |
| Exit | **Esc** opens the exit menu: **E** ends the flight (after 5 s), **Esc** again resumes |

**Cold start from the ramp** (Ramp start; *Realistic* avionics)
1. Main power on (**Ctrl+Alt+F2** steps it up).
2. JFS start (**Shift+J**). When the RPM rises, bring the throttle forward out of cut-off to idle.
3. Avionics power on. Set the INS to align (about 8 minutes for a full alignment).
4. Turn on the sensors (radar, radar altimeter) and the HUD.
5. Arm the ejection seat.
6. Set up countermeasures and the RWR.
7. Request taxi from the tower (**T**). Taxi to the hold-short line, then request take-off.

Use **Shift+P** (freeze) to take your time with the panel.

**Turning and energy**
- Corner speed is about **330–440 kt**, where the turn rate is greatest.
- The airframe is limited to 9 g; with blackout on, sustained g narrows your vision and leads to GLOC.
- Fast turns are wide, and slow turns are tight but bleed energy you cannot get back quickly.

**Deep stall** (Accurate flight model) shows as AOA pegged near 30°, or −5° inverted, with no
authority. To recover:
1. Release the stick and set idle.
2. Apply opposite rudder (inverted stall).
3. Engage manual pitch override (MPO) and pump the stick in phase with the nose oscillation.

**Landing**
1. From about 10 nm, line the flight-path marker up with the runway.
2. Gear down below 300 kt.
3. Fly the final approach at **11° AOA**.
4. Call the tower first (**T** → Request landing); an unauthorised landing is reported against you.
5. For instrument approaches, tune TACAN/ILS on the ICP and fly the HSI's course and glideslope.

After landing:
1. Engage nosewheel steering (**Shift+/**) and taxi clear.
2. Shut down: disarm the seat, avionics off, main power off, canopy open.

**Fuel and emergencies**

| Action | Keys |
|---|---|
| Dump fuel | **Alt+D** |
| Jettison everything | **Ctrl+J** |
| EPU | **Alt+E** steps the switch |
| Warning reset | **Ctrl+Alt+Shift+W** |

---

## 10. Sensors and weapons

**Master modes**

| Mode | Keys |
|---|---|
| NAV | **Shift+NumEnter**, or the ICP NAV button |
| Air-to-air | **Shift+Num0** |
| Air-to-ground | **Shift+Num.** |
| Dogfight override (radar to ACM, Sidewinders up) | **D** |
| Missile override (BVR set-up) | **M** |
| Cancel either override | **C** |

**Weapon selection**

| Action | Keys |
|---|---|
| Cycle A-A stations | **Enter** |
| Cycle A-G stations | **Backspace** |
| Step the A-G sub-mode (CCRP / CCIP / DTOS, …) | **'** (apostrophe) |
| Missile step (in the air) | **Shift+/** (nosewheel steering on the ground) |
| Uncage a Sidewinder | **U** |
| Master arm on / safe / cycle | **Shift+M** / **Ctrl+M** / **Ctrl+Alt+Shift+M** |
| Laser arm | **Alt+L** |
| Fire the gun | **/**, or joystick button 1. Ctrl+/ is the first trigger detent, Alt+/ the second. |
| Pickle (bombs, missiles) | **Space**, or joystick button 2 |

**Radar (FCR)**

| Action | Keys |
|---|---|
| A-A modes / A-G modes | **F1** / **F2** |
| Range down / up | **F3** / **F4** |
| Antenna tilt down / centre / up | **F5** / **F6** / **F7** |
| Gain down / up | **Shift+F3** / **Shift+F4** |
| Cursor enable / move | **Shift+N** / the arrow keys |
| Designate | **Num0** |
| Return to search | **Num.** |
| Lock next target | **PgDn** |

**HOTAS on the keyboard**

| Switch | Keys |
|---|---|
| TMS (target management) | **Ctrl+Up/Down/Left/Right** |
| DMS (display management) | **Shift+Num8/4/6/2** (see the Linux note in [section 2](#2-installing-and-starting)) |
| Pinky (sensor field of view) | **Alt+V** |

**Defensive systems**

| Action | Keys |
|---|---|
| Chaff | **X** |
| Flares | **Z** |
| ECM | **J** |
| RWR handoff | **Shift+PgDn** |

If you drop chaff but no flares, check the EWS program; programs 2 and 4 drop both (FF companion
note).

**HUD**

| Action | Keys |
|---|---|
| Declutter | **H** |
| HUD FLIR picture (this is the "black box" over the HUD some users report) | **Shift+H** |
| Colour | **Ctrl+Alt+Shift+C** |
| Pitch ladder | **Ctrl+Alt+Shift+P** |
| Flight-path marker | **Ctrl+Alt+Shift+F** |
| Brightness | **Ctrl+Alt+Shift+X** |

**Air-to-ground in brief**

| Weapon | How it works |
|---|---|
| **CCIP** (dumb bombs, rockets, guns) | Fly the pipper onto the target and pickle |
| **CCRP** | Designate the target (GM radar, or a targeting pod), fly the steering line, and hold the pickle until release |
| **LGB** | CCRP release with the laser armed (**Alt+L**) and the pod lasing the target |
| **JDAM / JSOW** (GPS) | Fall to the designated point or the pre-planned target. On this build, two JDAMs released in the *32 JDAMs* training mission guided along the line to their aim point and burst (2026-10-01). Released without a designation, a JDAM has no aim point; make sure you have a designated target first. |
| **Maverick / HARM** | Through the WPN page; see the Allied Force manual, chapter 5 |

The training missions (section 7) walk through each weapon.

---

## 11. Views

The number row selects views. The number pad pans and zooms.

**Cockpit views**

| Key | View |
|---|---|
| **1** | HUD only (MFDs can be overlaid) |
| **2** | 2-D cockpit (clickable switches); **Shift+2** toggles the wide angle |
| **3** | Virtual cockpit (3-D); **Shift+3** toggles the SA bar |
| **4** | Padlock: **Shift+4** air-to-air, **Alt+4** air-to-ground. **Num+** / **Num-** next / previous target (Shift for AA, Alt for AG). |
| **5** | Extended FOV: **Shift+5** / **Alt+5** for AA / AG |

**Outside views**

| Key | View |
|---|---|
| **6** | Tracking (you and your target) |
| **Shift+6** | Target |
| **7** | Incoming missile |
| **Shift+7** | Your weapon |
| **Alt+7** | Your weapon's target |
| **8** / **Shift+8** | Friendly aircraft / friendly ground unit |
| **Ctrl+8** / **Alt+8** | Enemy aircraft / enemy vehicle |
| **9** / **Shift+9** | Chase / fly-by |
| **0** | Orbit |
| **`** | Satellite |
| **Shift+`** | Action camera |
| **Shift+.** / **Shift+,** | Next / previous aircraft |

**Moving the view**
- **Num8 / 2 / 4 / 6** rotate the view; **Num1** zooms in and **Num7** zooms out.
- **Num9** glances forward and **Num3** glances back (while held).
- **L** gives a closer look.
- **Ctrl+Alt+Shift** with the number pad jumps to fixed cockpit views: **Num7** 10 o'clock, **Num9**
  2, **Num4** 9, **Num6** 3, **Num1** 8, **Num3** 4, **Num0** lower left, **Num2** lower right,
  **Num/** HUD, **Num5** reset.
- **Alt+C** then **L** loads your saved cockpit view set-up, and **Alt+C** then **S** saves it.

**Aids**
- **Labels:** **Shift+L** for nearby objects, then **Ctrl+L** adds distant ones. Labels must be
  enabled in Setup.
- **N** turns night-vision goggles on and off.
- **Ctrl+X** then **W** turns clouds on and off.

---

## 12. Radio

Each radio menu has a key. Pressing the key again pages forward; Shift plus the key pages back. The
number keys choose an entry.

| Key | Talk to | Highlights |
|---|---|---|
| **Q** | AWACS / FAC | Picture, Declare (identify your radar lock), Request help, Check in/out (CAS), Request relief, Rescue. Vectors page: nearest threat, target, package, tanker, divert, home plate. FAC page: FAC in/out, check in, request target, BDA. |
| **W** | Your wingman | Attack my target, weapons free/hold, check six, clear my six, rejoin, attacks (single-side offset, pince, posthole, chainsaw), resume mission, RTB, radar standby/active, say position/damage/status/fuel, formations (spread, arrowhead, box, res cell, wedge, trail, ladder, stack, kickout, close up), smoke and ECM on/off |
| **E** | The other element | as for W |
| **R** | The whole flight | as for W |
| **T** | Tower | Inbound, request landing, declare emergency, abort approach, landing number, request taxi, request take-off |
| **Y** | Tanker | Request fuel, ready to take fuel, done |

| Other radio keys | Effect |
|---|---|
| **Shift+F** | Checks in with the FAC directly |
| **Alt+Y** | Cycles the radio channel |
| **Shift+T** | Opens the chat line in multiplayer |

Your AI wingmen hold their fire until you say **weapons free** or give an attack call. After a
carrier launch, call **Rejoin**, or they head for a land base. On an OCA strike against an airbase,
a menu page lets each flight member claim a primary (+n) and secondary (−n) feature.

---

## 13. Carrier operations

FreeFalcon carriers have catapults, working blast deflectors and correct launch spots:
- Kuznetsov and Invincible have one spot.
- Essex, Forrestal and Charles de Gaulle have two.
- Reagan, Nimitz, Kennedy, America and Constellation have four.

You enter the world at take-off time on a launch spot, facing forward.

**Launch**
1. **Lower the hook** (**Ctrl+K**). On a carrier this means *attach to the catapult*: "Attached to
   Catapult" appears, and the blast deflector rises behind you (except on Essex and Invincible).
2. Go to full power.
3. **Raise the hook** to fire the catapult.
4. Check flaps when heavy; the F-4B/C/J and Super Étendard are sensitive.

**Recovery** (summarised from the companion's carrier guide)
1. Send your wingmen home once you are feet-wet; the AI does not land on carriers.
2. Ask AWACS for vectors to the carrier group (**Q**); it also gives the carrier's TACAN channel.
3. On the ICP: T-ILS, enter the channel, ENTR, then DCS up and **0** to switch the band to Y.
4. Find your aircraft's slowest stable approach speed at 3–5° AOA (flaps, speed brakes and gear out)
   before the approach.
5. Fly the approach and catch a wire with the hook down.

---

## 14. Multiplayer

On Linux, FreeFalcon uses UDP ports **2934–2937**:
- 2934/2935 for the game;
- 2936/2937 for optional voice.

The host must allow them in (`sudo ufw allow 2934:2937/udp`); over the internet they also need
port forwarding. Use the same build on both PCs. Give the two pilots different callsigns.

**Connect**
- **Host:** Main menu → **COMMS**. Leave *URL/IP* as it is, choose **Connect as Server**, press
  **Connect**. Then close the connection-status window with **Cancel**, or later clicks land on it.
- **Joiner:** **COMMS** → type the host's IP (`ip:port` also works; the default port is 2934) → **Connect
  as Client** → **Connect** → close the status window.
- **Shortcut:** start the host with `FF_MP_CONNECT=2934` and the joiner with
  `FF_MP_CONNECT=2934:2934:<host-ip>`, and both skip the COMMS screen.

**Dogfight** (the most reliable mode)
1. The host creates the game.
2. The joiner picks it in the list, commits, and accepts the info window.
3. Both right-click in the team list → **Join**, then **Fly**.

**Tactical Engagement**
1. The host commits a TE.
2. The joiner picks it under the online list and commits.
3. Each picks a flight that is still in Briefing, clicks a pilot slot, and presses **Fly**.

Choose a TE whose flights survive loading: **Sink the Kuz** works, *Havin' Fun Strike* does not (see
section 7).

**Campaign**
1. The host enters a campaign; committing is what hosts it.
2. The joiner uses the **JOIN** tab, clicks `<host callsign>'s Game` (click the name itself), then
   **Commit** and **Comply**.
3. Both pick a flight and **Fly**. Both may take the same flight.

The host's Fly stays grey until the host has a flight. The clock runs at the slowest speed either
player asks for.

**Rules of engagement.** The host sets the minimum realism for joiners. A joiner whose options are
out of range sees **Comply** instead of OK. In the chat window you can mute or ignore players, see
their logbook, or jump to their flight.

**Two copies on one PC**
1. Copy the data: `cp -a ~/.local/share/freefalcon/FreeFalcon6 ~/ff2` (14 GB).
2. Host: `FF_MP_CONNECT=2934 ./FreeFalcon-….AppImage`
3. Joiner: `FF_GAMEDATA=~/ff2 FF_MP_CONNECT=2944:2934:127.0.0.1 ./FreeFalcon-….AppImage`
4. Put the windows on different monitors.

**State of play (2026-10-01)**
- **Working (two PCs, 09-19):** joining, dogfight teams, and a shared flight with weapons.
- **Working (one PC since):** damage across the wire, the campaign clock, the campaign joiner reaching
  take-off, and a TE joiner reaching the cockpit (*Sink the Kuz*).

**For a bug report**
- Run both sides with `FF_DEBUG_MPCOMMS=1 FF_DEBUG_MPMSG=1`, and add `FF_DEBUG_STARTCAMP=1` if the
  joiner can't take off.
- On the host, `ss -lun | grep 2934` shows whether it is listening.

---

## 15. Keyboard reference

The complete, machine-generated map is **[`KEYMAP.md`](KEYMAP.md)**:
- 458 key bindings and 8 joystick buttons, read from `config/keystrokes.key`, the file the default
  pilot loads (the fallback when a pilot's own file is missing);
- each binding is checked against the program's command table, which shows 4 dead bindings and 453
  commands that have no default key.

Regenerate it with:

```bash
python3 tools/ff_keymap.py <data>/config/keystrokes.key src/sim/siminput/findfunc.cpp > docs/KEYMAP.md
```

**How the key file works** (`src/sim/siminput/setupinp.cpp`)
- Each line is `Command button mouseSide key mod prefixKey prefixMod flags "description"`.
- Keys are DirectInput scancodes. The modifier is a bitmask: Shift 1, Ctrl 2, Alt 4.
- A **prefix key** makes a two-key chord, for example *Alt+C, then L*.
- A prefix of −1 is a radio-menu binding: the digits 1–0 (and Esc) act *after* the menu key.
- A key of −2 is a joystick button. A key of −1 is not loaded at all.
- A command name the program does not know is silently ignored. That is true of `OTWTextureDecrease`,
  `OTWTextureIncrease`, `OTWToggleAlpha` and `OTWToggleRoof` in the shipped file.

**Joystick buttons by default**

| Button | Command |
|---|---|
| 1 | Gun trigger |
| 2 | Pickle |
| 3 | Designate |
| 4 | Drop track |
| 5 | Next A-A weapon |
| 6 | Next A-G weapon |
| 7 | Next nav mode |
| 8 | FCC sub-mode step |

**The keys you will use most**

| Flight | | Fight | | Views and radio | |
|---|---|---|---|---|---|
| Throttle | = / - , Alt+- idle | Gun / pickle | / / Space | Cockpit 2-D / 3-D | 2 / 3 |
| Afterburner | Ctrl+= / Ctrl+- | A-A / A-G stations | Enter / Backspace | Padlock / EFOV | 4 / 5 |
| Gear / speed brake | G / Shift+B, Ctrl+B | Dogfight / MRM / cancel | D / M / C | Chase / orbit | 9 / 0 |
| Wheel / parking brake | K / Alt+P | Chaff / flare / ECM | X / Z / J | Satellite | ` |
| Rudder | , / . | Radar A-A / A-G mode | F1 / F2 | AWACS / wingman / flight | Q / W / R |
| Autopilot | A | Radar range | F3 / F4 | Tower / tanker | T / Y |
| Pause / freeze | P / Shift+P | TMS | Ctrl+arrows | Labels | Shift+L |
| Time 2× / 4× | Tab / CapsLock | Master arm | Shift+M | Exit menu | Esc |

---

## 16. Corrections to the older manuals

The Allied Force manual and the community notes disagree with the shipped FreeFalcon key file here:

| Function | Older document | FreeFalcon (program) |
|---|---|---|
| Elevator / roll trim | Alt+arrows (AF ch. 16) | **Alt+Shift+Home / End / Delete** |
| DMS | Ctrl+Num8/2/4/6 (AF ch. 16) | **Shift+Num8/2/4** (DMS right is Shift+Num6, OTWStepMFD2) |
| AP disconnect | Ctrl+3 | **Ctrl+4** |
| Satellite view | Alt+` | **`** |
| Glance forward | Num1 | **Num9**. Num1 is zoom in. |
| Enemy view | Shift+6 | **Ctrl+8** (enemy aircraft), **Alt+8** (enemy vehicle). Shift+6 is the target view. |
| Radio top-level menu | Tab | **Tab is 2× time compression.** Use the individual menu keys Q/W/E/R/T/Y. |
| Exit | Esc, then E | Same: **Esc** opens the exit menu, **E** ends the flight |

The `overview.md` notes in the DOC folder describe **Falcon BMS 4.32**, a different program with
different keys. Don't use them for FreeFalcon.

---

## 17. ACMI and screenshots

**ACMI.** **F** toggles the in-flight recorder (AVTR); the Setup file-size limit starts a new tape
when it is reached. **ACMI** on the main menu replays a tape with satellite and isometric views,
labels, and wing trails for studying a fight.

**Screenshots**

| Key | Effect |
|---|---|
| **PrintScreen** | A plain screenshot |
| **Shift+O** | A "pretty" screenshot |
| **Ctrl+Z** then **R** | Frame-rate display |

---

## 18. Troubleshooting

| Symptom | What to do |
|---|---|
| First start is slow | The 14 GB data copy happens once. `FF_GAMEDATA` points at an existing install instead. |
| A training or TE list shows the wrong missions | The lists follow the **current theatre**; pick Korea in Theatre. |
| A TE has nothing to fly | Some scenarios cancel their own flights at load (*Havin' Fun Strike*). Use another, such as *Sink the Kuz*. |
| Black rectangle over the HUD | That is the HUD FLIR picture; **Shift+H** turns it off. |
| Pressing DMS on the keyboard changes the 2-D panel | Known Linux-build issue (FF-KEYPANEL-1): Shift+Num8/4/6/2 also select a panel. Use joystick buttons for DMS, or stay in the 3-D cockpit. |
| Joiner cannot connect | Check `ss -lun \| grep 2934` on the host and the firewall rule, and close the connection-status window on both sides. |
| Two copies on one PC behave oddly | Separate data folders (`FF_GAMEDATA`), ports 10 apart (`FF_MP_CONNECT`), and separate monitors. |
| A key from an old guide does nothing | Look it up in [`KEYMAP.md`](KEYMAP.md); four shipped bindings are dead, and many guides describe other Falcon versions. |
