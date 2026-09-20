#!/usr/bin/env bash
# RECON-1 A/B on the PO's path: recon view from the steerpoint popup, then the right-hand rotate caret twice.
# Compare terrain vs object rotation direction; fix (default) vs FF_NO_FBO_YFLIP=1.
/tmp/claude-1001/-home-admin/679c6939-a4c2-41fb-8e58-365614e5ffb5/scratchpad/po_idle.sh
cd /home/admin/freefalcon-central
base="677,748@12;140,416@18;824,748@24;532,320@40r;549,356@45"
for arm in fix_base fix_rot nofix_base nofix_rot; do
  clicks="$base"; [ ${arm#*_} = rot ] && clicks="$base;381,750@54;381,750@57"
  extra=""; [ ${arm%_*} = nofix ] && extra="-e FF_NO_FBO_YFLIP=1"
  FF_VAL_OUT=$HOME/ff-gates/recon gl-lock -w 3600 bash tools/ff_validate.sh caret_$arm -m ui -t 62 -r 68 -c "$clicks" -e FF_DEBUG_RECON=1 $extra > $HOME/ff-gates/recon/caret_$arm.out 2>&1
  echo "$arm: $(tail -2 $HOME/ff-gates/recon/caret_$arm.out | tr '\n' ' ' | cut -c1-140)"
done
echo FF-RECON-CARET-done
