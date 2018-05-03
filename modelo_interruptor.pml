mtype ={ON, OFF};

bit p;

active proctype lampara_fsm (){

  byte state = OFF;
  do
  ::(state == OFF) ->
    if
    ::p -> state = ON; p = 0;
    fi;
  ::(state == ON) ->
    if
    ::p -> state = OFF; p = 1;
    fi;
  od;

};

active proctype entorno (){

  do
  ::p=1;
  ::p=0;
  od;

};

ltl spec {

  []((state == ON) && (p -> (<>state == OFF))) &&
  []((state == OFF) && (p -> (<>state == ON)));

};
