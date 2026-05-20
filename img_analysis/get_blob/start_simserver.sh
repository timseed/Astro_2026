#!/bin/bash
#
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
INDISVR="indiserver"
PIPE="/tmp/indiFIFO"
if pgrep -x $INDISVR >/dev/null; then
  echo "$INDISVR is Running"
  echo "Manually kill this to start $INSISVR again"
  echo ""
  echo ""
  echo "killall $INDISVR in 4 seconds !! "
  sleep 4
  PID=$(pgrep $INDISVR)
  echo "It has a PID of $PID"
  #exit 0
  kill "$PID"
else
  echo "$INDISVR is Stopped"
fi

if [[ -p "$PIPE" ]]; then
  echo "$PIPE exists."
  rm -f $PIPE
  echo "$PIPE has been removed"
else
  echo "$PIPE does not exist."
fi

# Now create the pipe
mkfifo $PIPE
nohup $INDISVR -vv -f $PIPE 2>&1 >~/indisvr_log.log &
# need a rest (and let indi to calm down)
sleep 4
echo "start indi_simulator_telescope " >>$PIPE
echo "Started indi_simulator_telescope process."
sleep 2
echo "start indi_simulator_ccd " >>$PIPE
echo "Started indi_simulator_ccd process."
sleep 2
echo "start indi_simulator_guide " >>$PIPE
echo "Started indi_simulator_guide process."
sleep 2
echo "start indi_simulator_gps " >>$PIPE
echo "Started indi_simulator_gps process."
sleep 2
echo "Completed"
echo "================="
echo "= Indi Status   ="
echo "================="
ps -ef | grep indi | grep -v grep
echo "Set GPS Position is fixed to Arenas, PH"
echo "Checking"
indi_getprop | grep COORD
echo "Check AAF with CCD "
indi_getprop | grep SV405CC.ACTIVE_DEVICES.ACTIVE_FOCUSER
tail -f ~/indisvr_log.log
