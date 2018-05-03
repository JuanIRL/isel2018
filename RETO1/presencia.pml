ltl spec {
     [] (((state == OFF) && presencia) -> <> (state == ON)) &&
     [] (((state == ON) && timeout) -> <> (state == OFF))
 }
mtype = {ON, OFF};

 mtype state;
 bit presencia;
 active proctype lampara_fsm () {
        state = OFF;
        do
        :: (state == OFF) -> atomic {
           if
           :: presencia -> state = ON; presencia = 0;
           fi;
        }
        :: (state == ON)  -> atomic {
           if
           :: timeout -> state = OFF;
           fi;
        }
        od;
 }
 active proctype entorno () {
        do
        :: presencia = 1
        :: (! presencia) -> skip
        od;
 }
