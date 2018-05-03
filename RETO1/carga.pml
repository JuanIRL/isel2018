ltl spec {
     [] ((seccion == 0) -> <> (state == OFF)) &&
     [] ((state == CARGA && carga_aum) -> <> (state == ABRIR_SECCION)) &&
     [] ((state == CARGA && carga_dis) -> <> (state == CERRAR_SECCION))
}
mtype = {OFF, CARGA, ABRIR_SECCION, CERRAR_SECCION};

 mtype state;
 bit carga_aum;
 bit carga_dis;
 byte seccion;

 active proctype carga_fsm () {
        state = OFF;
        seccion = 0;
        do
        :: (state == OFF) -> atomic {
           if
           :: (carga_aum) -> state = CARGA;seccion = 1;carga_aum = 0; carga_dis = 0;
           fi;
        }
        :: (state == CARGA)  -> atomic {
           if
           :: (seccion == 0) -> state = OFF;carga_aum = 0; carga_dis = 0;
           :: ((seccion > 0) && carga_aum) -> state = ABRIR_SECCION;
           :: ((seccion > 0) && carga_dis) -> state = CERRAR_SECCION;
           fi;
        }
        :: (state == ABRIR_SECCION)  -> atomic {
           if
           :: state = CARGA;carga_aum = 0; carga_dis = 0; seccion++;
           fi;
        }
        :: (state == CERRAR_SECCION)  -> atomic {
           if
           :: state = CARGA;carga_aum = 0; carga_dis = 0; seccion--;
           fi;
        }
        od;
 }
 active proctype entorno () {
        do
        :: carga_aum = 1; carga_dis = 0;
        :: carga_aum = 0; carga_dis = 1;
        :: carga_aum = 0; carga_dis = 0;
        od;
 }
