ltl spec {
     [] (((state == OFF) && carga_aum) -> <> (state == EXTERIOR || state == INTERIOR)) &&
     [] (((state == INTERIOR) && entorno) -> <> (state == EXTERIOR)) &&
     [] (((state == EXTERIOR) && !entorno) -> <> (state == INTERIOR))&&
     [] (((state != OFF) && !seccion) -> <> (state == OFF))
 }
mtype = {EXTERIOR, INTERIOR, OFF};

 mtype state;
 bit carga_aum;
 bit entorno;
 bit seccion;

 active proctype airfreecooling_fsm () {
        state = OFF;
        do
        :: (state == OFF) -> atomic {
           if
           :: (carga_aum && entorno) -> state = EXTERIOR;seccion = 1;
           :: (carga_aum && !entorno)-> state = INTERIOR;seccion = 1;
           fi;
        }
        :: (state == INTERIOR)  -> atomic {
           if
           :: (entorno && seccion) -> state = EXTERIOR;
           :: (!seccion && !carga_aum) -> state = OFF;
           fi;
        }
        :: (state == EXTERIOR)  -> atomic {
           if
           :: (!entorno && seccion) -> state = INTERIOR;
           :: (!seccion && !carga_aum) -> state = OFF;
           fi;
        }
        od;
 }
 active proctype ent () {
        do
        :: carga_aum = 1
        :: (! carga_aum) -> skip
        ::entorno = 1
        :: (! entorno) -> skip
        :: seccion = 1
        :: (! seccion) -> skip
        od;
 }
