/*
** Compilador de C para transputer.
** Preprocesador y funciones varias.
**
** por Oscar Toledo Gutiérrez.
**
** (c) Oscar Toledo G.1995.
**
** Creación: 1 de junio de 1995.
** Revisión: 25 de julio de 1995. Nueva función, isspace().
** Revisión: 25 de julio de 1995. Se agrega #ifdef, #ifndef, #else y #endif
**                                al preprocesador.
** Revisión: 26 de julio de 1995. Nueva función. encuentra().
** Revisión: 26 de julio de 1995. Se agrega #undef y #line al preprocesador.
**                                Nueva función. borra_macro().
**                                Modifico busca_macro() para que retorne
**                                la posición del principio de la macro.
** Revisión: 26 de julio de 1995. Se agrega #if.
** Revisión: 26 de julio de 1995. Se pasa #define y #include al preprocesador.
** Revisión: 26 de julio de 1995. Se pasa #asm al preprocesador.
** Revisión: 27 de julio de 1995. Soporte para concatenar líneas usando \.
** Revisión: 27 de julio de 1995. Nuevas funciones. strcpy(), strcat().
** Revisión: 27 de julio de 1995. Ahora soporta #include y #define estandard.
**                                El preprocesador ya esta casí completo,
**                                faltan los #include anidados.
** Revisión: 27 de julio de 1995. Corrección de un defecto en encuentra().
** Revisión: 10 de agosto de 1995. Nueva función. lee_entero().
** Revisión: 10 de agosto de 1995. Modificación de nueva_glb y nueva_loc,
**                                 para el soporte de tipos complejos.
** Revisión: 10 de agosto de 1995. Nueva función. escribe_entero().
** Revisión: 12 de agosto de 1995. Soporte para #include anidado.
** Revisión: 23 de agosto de 1995. Nuevas funciones, busca_miembro(),
**                                 nuevo_miembro(), busca_estructura(),
**                                 nueva_estructura().
** Revisión: 24 de agosto de 1995. Nuevas funciones, busca_enum(),
**                                 nuevo_enum().
** Revisión: 29 de noviembre de 1995. Se pasa la funcion car_act() al archivo
**                                    CCVARS.C
** Revisión: 27 de diciembre de 1995. Corrección de un defecto horrible en el
**                                    preprocesamiento de macros con pars.
** Revisión: 28 de diciembre de 1995. Corrección de un error que hacia que
**                                    quedara un archivo abierto al salir por
**                                    un error.
** Revisión: 20 de junio de 1996. Hago que nueva_glb() y nueva_loc agregen
**                                el nivel de profundidad.
*/

/*
** Función para cálcular la dispersión PJW (P.J. Weinberger)
**
** Tomada del libro "Compiladores: Principios, técnicas y herramientas"
** Alfred V. Aho, Ravi Sethi, Jeffrey D. Ullman. Addison-Wesley 1990.
** Páginas 450-452.
*/
int calcula_dispersion(cadena)
  char *cadena;
{
  int val = 0, temp;

  while (*cadena) {
    val = (val << 4) + *cadena++;
    if (temp = val & 0xf0000000) {
      val ^= temp >> 24;
      val &= ~0xf0000000;
    }
  }
  return val % NUM_PRIMO;
}

/*
** Un nuevo bucle, lo agrega al final de la lista enlazada
*/
void nuevo_bucle(ap)
  struct bucle *ap;
{
  ap->anterior = ultimo_bucle;  /* Bucle anterior */
  ap->pila = pila;              /* Nivel de la pila */
  ap->bucle = nueva_etiq;       /* Etiqueta del bucle */
  ap->fin = nueva_etiq;         /* Etiqueta de salida */
  ultimo_bucle = ap;
}

/*
** Una nueva variable/función global.
*/
struct nombres *nueva_glb(nombre, id, clase, tipo, valor)
  unsigned char *nombre, *tipo;
  int valor, clase, id;
{
  struct nombres *ap;
  unsigned char *ap1;
    
  if (ap_glb >= globales + NUM_GLBS) {
    error("Global table full");
    return 0;
  }
  ap1 = ap_glb->nombre;
  while (alfanum(*ap1++ = *nombre++));  /* Copia el nombre */
  ap = ap_glb;
  ap->ident = id;
  ap->clase = clase;
  ap->nivel = 0;
  ap->tipo = tipo;
  ap->posicion = valor;
  ap_glb++;
  return ap;
}

/*
** Una nueva variable local.
*/
struct nombres *nueva_loc(nombre, id, clase, tipo, valor)
  unsigned char *nombre, *tipo;
  int valor, clase, id;
{
  struct nombres *ap;
  unsigned char *ap1;

  if (ap_loc >= locales + NUM_LOCS) {
    error("Local table full");
    return 0;
  }
  ap1 = ap_loc->nombre;
  while (alfanum(*ap1++ = *nombre++));  /* Copia el nombre */
  ap = ap_loc;
  ap->ident = id;
  ap->clase = clase;
  ap->nivel = nivel;
  ap->tipo = tipo;
  ap->posicion = valor;
  ap_loc++;
  return ap;
}

/*
** Una nueva estructura (en realidad un rótulo de estructura).
*/
struct rotulo *nueva_estructura(nombre)
  char *nombre;
{
  struct rotulo *ap;

  ap = malloc(sizeof(struct rotulo) + strlen(nombre));
  if (ap == NULL) {
    error("Out of memory");
    return NULL;
  }
  if (lista_estructura == NULL)
    lista_estructura = ap;
  if (ultima_estructura != NULL)
    ultima_estructura->sig = ap;
  ultima_estructura = ap;
  ap->sig = NULL;
  ap->es_union = 0;
  ap->que_es = 0;
  ap->lista = NULL;
  ap->tam = 0;
  strcpy(ap->nombre, nombre);
  return ap;
}

/*
** Un nuevo miembro de una estructura.
*/
struct miembro *nuevo_miembro(lista, nombre)
  struct miembro **lista;
  char *nombre;
{
  struct miembro *nuevo, *sig;

  nuevo = malloc(sizeof(struct miembro) + strlen(nombre));
  if (nuevo == NULL) {
    error("Out of memory");
    return NULL;
  }
  if (*lista == NULL)
    *lista = nuevo;
  else {
    sig = *lista;
    while (sig->sig != NULL)
      sig = sig->sig;
    sig->sig = nuevo;
  }
  nuevo->sig = NULL;
  nuevo->tipo = NULL;
  nuevo->posicion = 0;
  strcpy(nuevo->nombre, nombre);
  return nuevo;
}

/*
** Un nuevo enumerador.
*/
void nuevo_enum(nombre, valor)
  char *nombre;
  int valor;
{
  struct enumerador *nuevo;
  int dispersion;

  nuevo = malloc(sizeof(struct enumerador) + strlen(nombre));
  if (nuevo == NULL) {
    error("No hay memoria");
    return;
  }
  dispersion = calcula_dispersion(nombre);
  nuevo->sig = tabla_enum[dispersion];
  tabla_enum[dispersion] = nuevo;
  nuevo->valor = valor;
  strcpy(nuevo->nombre, nombre);
}

/*
** Busca una variable/función global.
*/
struct nombres *busca_glb(nombre)
  unsigned char *nombre;
{
  struct nombres *ap;

  ap = globales;
  while (ap != ap_glb) {
    if (astreq(nombre, ap->nombre, MAX_NOMBRE))
      return ap;
    ap++;
  }
  return NULL;
}

/*
** Busca una variable/función local.
*/
struct nombres *busca_loc(nombre)
  unsigned char *nombre;
{
  struct nombres *ap;

  ap = ap_loc;
  while (ap != locales) {
    ap--;
    if (astreq(nombre, ap->nombre, MAX_NOMBRE))
      return ap;
  }
  return NULL;
}

/*
** Busca una estructura.
*/
struct rotulo *busca_estructura(nombre)
  unsigned char *nombre;
{
  struct rotulo *ap;

  ap = lista_estructura;
  while (ap != NULL) {
    if (astreq(nombre, ap->nombre, MAX_NOMBRE))
      return ap;
    ap = ap->sig;
  }
  return NULL;
}

/*
** Busca un miembro de estructura.
*/
struct miembro *busca_miembro(lista, nombre)
  struct miembro *lista;
  unsigned char *nombre;
{
  while (lista != NULL) {
    if (astreq(nombre, lista->nombre, MAX_NOMBRE))
      return lista;
    lista = lista->sig;
  }
  return NULL;
}

/*
** Busca un enumerador.
*/
struct enumerador *busca_enum(nombre)
  unsigned char *nombre;
{
  struct enumerador *ap;

  ap = tabla_enum[calcula_dispersion(nombre)];
  while (ap != NULL) {
    if (astreq(nombre, ap->nombre, MAX_NOMBRE))
      return ap;
    ap = ap->sig;
  }
  return NULL;
}

/*
** Checa si la proxima cadena de entrada es un nombre legal.
*/
int nombre_legal(nombre)
  unsigned char *nombre;
{
  int k;
  unsigned char c;

  espacios();
  if (letra(car_act) == 0)
    return (*nombre = 0);
  k = 0;
  while (alfanum(car_act)) {
    if(k < MAX_NOMBRE)
      nombre[k++] = obt_car();
    else
      obt_car();
  }
  nombre[k] = 0;
  return 1;
}

/*
** Imprime un retorno de carro y una cadena a la consola.
*/
void mensaje(cad)
  unsigned char *cad;
{
  puts("\n");
  puts(cad);
}

/*
** Siguiente caracter en la línea.
*/
int prox_car()
{
  if (car_act == 0)
    return 0;
  else
    return linea[pos_linea + 1];
}

/*
** Pasa al siguiente caracter, retorna el caracter anterior.
*/
int obt_car()
{
  if (car_act == 0)
    return 0;
  else
    return linea[pos_linea++];
}

/*
** Descarta la línea actual.
*/
void descarta()
{
  pos_linea = 0;
  linea[pos_linea] = 0;
}

/*
** Obtiene un caracter, si era el último en la línea, carga
** otra línea.
*/
int lee_car()
{
  while (car_act == 0) {
    if (eof)
      return 0;
    preprocesa();
  }
  return obt_car();
}

/*
** Obtiene otra linea de la entrada.
*/
void lee_linea()
{
  int k;

  while (1) {
    descarta();
    if (entrada == 0) {
      eof = 1;
      return;
    }
    while ((k = fgetc(entrada)) > 0) {
      if (k == 13)
        continue;
      if ((k == '\n') || (pos_linea >= MAX_LINEA))
        break;
      linea[pos_linea++] = k;
    }
    linea[pos_linea] = 0;   /* Agrega un caracter nulo */
    linea_actual++;         /* Se ha leido una línea más */
    if (k <= 0) {
      if (nivel_incl)
        fin_include();
      else {
        fclose(entrada);
        entrada = 0;
      }
    }
    if (pos_linea) {
      if (intercala_fuente) {
        comentario();
        emite_texto(linea);
        emite_nueva_linea();
      }
      pos_linea = 0;
      return;
    }
  }
}

/*
** Hace el preprocesamiento.
*/
void preprocesa()
{
  int k, car, hay_if;
  unsigned char c, nombre[TAM_NOMBRE], *def, *busqueda;
  int subs, pars, args, paren, m;

  hay_if = 0;
  dentro_pp = SI;
  while(1) {
    primer_paso();
    if (eof) {
      dentro_pp = NO;
      return;
    }
    espacios();
    if(car_act == '#') {
      pos_linea++;
      espacios();
      if(match("ifdef")) {
        ++nivel_if;
        if(evadir_nivel) continue;
        nombre_legal(nombre);
        if(busca_macro(nombre) == 0)
          evadir_nivel = nivel_if;
        continue;
      }
      if(match("ifndef")) {
        ++nivel_if;
        if(evadir_nivel) continue;
        nombre_legal(nombre);
        if(busca_macro(nombre))
          evadir_nivel = nivel_if;
        continue;
      }
      if(match("if")) {
        ++nivel_if;
        if(evadir_nivel) continue;
        hay_if = pos_linea;
        break;
      }
      if(match("else")) {
        if(nivel_if) {
          if(evadir_nivel == nivel_if)
            evadir_nivel = 0;
          else if(evadir_nivel == 0)
            evadir_nivel = nivel_if;
        } else error("No #if...");
        continue;
      }
      if(match("endif")) {
        if(nivel_if) {
          if(evadir_nivel == nivel_if)
            evadir_nivel = 0;
          --nivel_if;
        } else error("No #if...");
        continue;
      }
      if(evadir_nivel) continue;
      if(match("asm")) {
        while(1) {
          lee_linea();
          if(eof) {
            dentro_pp = NO;
            return;
          }
          espacios();
          if(car_act == '#')
            break;
          emite_linea(linea + pos_linea);
        }
        continue;
      }
      if(match("include")) {
        p_include();
        continue;
      }
      if(match("define")) {
        nueva_macro();
        continue;
      }
      if(match("undef")) {
        nombre_legal(nombre);
        borra_macro(nombre);
        continue;
      }
      if(match("line")) continue;
    }
    if(evadir_nivel) continue;
    break;
  }
  pos_linea = hay_if;
  subs = SI;
  while(subs) {
    subs = NO;
    pos_linea_m = 0;
    while (car = car_act) {
      if (isspace(car))
        pp_espacios();
      else if (car == '"')
        pp_comillas();
      else if (car == 39)
        pp_apostrofe();
      else if (letra(car)) {
        k = 0;
        while (alfanum(car_act)) {
          if (k < MAX_NOMBRE)
            nombre[k++] = car_act;
          obt_car();
        }
        nombre[k] = 0;
        if(k = busca_macro(nombre)) {
          m = 0;
          while (macs[k++]);
          pars = macs[k++];
          def = macs + k;
          if(pars) {
            espacios();
            if(car_act != '(') error("Missing (");
            obt_car();
            args = paren = 0;
            while(car_act && car_act != ')') {
              espacios();
              while(car_act) {
                if(car_act == ',' && paren == 0)
                  break;
                if(car_act == '(')
                  ++paren;
                if(car_act == ')') {
                  if(paren == 0) break;
                  --paren;
                }
                if(m < MAX_AMAC)
                  amacs[m++] = obt_car();
                else {
                  error("Macro parameter table is full");
                  cancela();
                }
              }
              amacs[m++] = 0;
              if(car_act == ',')
                obt_car();
              ++args;
            }
            if(args != pars)
              error("Wrong number of arguments");
            if(car_act != ')')
              error("Missing )");
            obt_car();
          }
          while(*def) {
            if(*def != 127) almacena_car(*def++);
            else {
              busqueda = amacs;
              k = *++def;
              while(--k)
                while(*busqueda++) ;
              while(*busqueda)
                almacena_car(*busqueda++);
              ++def;
            }
          }
          subs = SI;
        } else {
          k = 0;
          while(c = nombre[k++])
            almacena_car(c);
        }
      } else
        almacena_car(obt_car());
    }
    if (hay_if)                /* Si hay #if, agrega un ; para que no vaya */
      almacena_car(';');       /* a salirse de la linea en caso de error. */
    almacena_car(0);
    strcpy(linea, linea_m);
    pos_linea = 0;
    if (pos_linea_m >= MAX_LINEA) {
      error("Line too long");
      break;
    }
  }
  if (hay_if) {
    if(expr_constante() == 0)
      evadir_nivel = nivel_if;
    descarta();
  }
  dentro_pp = NO;
}

/*
** Primer paso del preprocesamiento, pega líneas terminadas en \, y
** elimina los comentarios.
*/
void primer_paso()
{
  int car;

  lee_linea();
  pos_linea = pos_linea_m = 0;
  if(eof) return;
  while (car = car_act) {
    if (isspace(car))
      pp_espacios();
    else if ((car == '\\') && (prox_car() == 0)) {
      lee_linea();
      if(eof) return;
    } else if (car == '"')
      pp_comillas();
    else if (car == 39)
      pp_apostrofe();
    else if ((car == '/') && (prox_car() == '*')) {
      almacena_car(' ');
      pp_comentarios();
    } else
      almacena_car(obt_car());
  }
  almacena_car(0);
  if (pos_linea_m >= MAX_LINEA)
    error("Line too long");
  pos_linea = pos_linea_m = 0;
  while (linea[pos_linea++] = linea_m[pos_linea_m++]);
  pos_linea = 0;
}

int almacena_car(c)
  unsigned char c;
{
  linea_m[pos_linea_m] = c;
  if (pos_linea_m < MAX_LINEA)
    pos_linea_m++;
  return c;
}

/*
** elimina espacios y tabuladores extras.
*/
void pp_espacios()
{
  almacena_car(' ');
  while (isspace(car_act))
    obt_car();
}

/*
** Procesa cadenas de caracteres.
*/
void pp_comillas()
{
  almacena_car(obt_car());
  while ((car_act != '"') ||
        ((linea[pos_linea - 1] == 92) && (linea[pos_linea - 2] != 92))) {
    if (car_act == 0) {
      error("Missing quotes");
      break;
    }
    almacena_car(obt_car());
  }
  obt_car();
  almacena_car('"');
}

/*
** Procesa caracteres encerrados entre '
*/
void pp_apostrofe()
{
  almacena_car(39);
  obt_car();
  while ((car_act != 39) ||
        ((linea[pos_linea - 1] == 92) && (linea[pos_linea - 2] != 92))) {
    if (car_act == 0) {
      error("Missing apostrophe");
      break;
    }
    almacena_car(obt_car());
  }
  obt_car();
  almacena_car(39);
}

/*
** Procesa y elimina comentarios.
*/
void pp_comentarios()
{
  pos_linea = pos_linea + 2;
  while ((car_act != '*') ||
         (prox_car() != '/')) {
    if (car_act == 0)
      lee_linea();
    else
      ++pos_linea;
    if (eof)
      break;
  }
  pos_linea = pos_linea + 2;
}

/*
** Añade una nueva macro en la tabla.
*/
void nueva_macro()
{
  unsigned char nombre[TAM_NOMBRE];
  int k, car;
  int num_args;
  int l;        /* indice en la tabla de argumentos de macros */
  int numero;
  unsigned char *busqueda;

  if (nombre_legal(nombre) == 0) {
    nombre_ilegal();
    descarta();
    return;
  }
  borra_macro(nombre);
  k = 0;
  while (pone_macro(nombre[k++]));
  num_args = l = 0;
  if(car_act == '(') {   /* genera una lista de nombres de parametros */
    obt_car();
    while(car_act != ')') {
      espacios();
      if(letra(car_act)) {
        while(alfanum(car_act)) {
          if(l < MAX_AMAC) amacs[l++] = obt_car();
          else {
            error("Macro parameter table is full");
            cancela();
          }
        }
        amacs[l++] = 0;
        num_args++;
      }
      espacios();
      if(car_act == ',') obt_car();
      else if(car_act != ')') {
        error("Missing ) in #define");
        break;
      }
    }
    obt_car();
    amacs[l++] = 0;
  }
  espacios();
  pone_macro(num_args);  /* Ahora substituye los nombres por */
  while(car_act) {     /* secuencias 0x7f num */
    if(num_args && letra(car_act)) {
      numero = 1;
      busqueda = amacs;
      while(*busqueda) {         /* rastrea nombres */
        k = 0;
        while(1) {
          if(busqueda[k] != linea[pos_linea + k]) break;
          if(linea[pos_linea + k] < ' ') break;
          k++;
        }
        if(alfanum(busqueda[k]) || alfanum(linea[pos_linea + k])) {
          ++numero;
          while(*busqueda++) ;
        } else {
          pos_linea += k;
          pone_macro(127);
          pone_macro(numero);
          break;
        }
      }
      if(*busqueda == 0)
        while(alfanum(car_act))
          pone_macro(obt_car());
    } else {
      pone_macro(obt_car());
    }
  }
  pone_macro(0);
  if (ap_mac >= MAX_MAC)
    error("Macro table full");
}

/*
** Elimina una macro de la tabla.
*/
void borra_macro(nombre)
  unsigned char *nombre;
{
  int k, l, m;

  if(k = busca_macro(nombre)) {    /* Obtiene el comienzo de la macro */
    l = k;
    while (macs[k++]);             /* Busca el comienzo de la siguiente */
    k++;                           /* Evade la cuenta de argumentos */
    while (macs[k++]);
    m = k - l;
    while (k != ap_mac)            /* Mueve el bloque hacia atras */
      macs[l++] = macs[k++];
    ap_mac = ap_mac - m;           /* Ahora hay más espacio libre */
  }
}

int pone_macro(c)
  unsigned char c;
{
  macs[ap_mac] = c;
  if (ap_mac < MAX_MAC)
    ap_mac++;
  return c;
}

/*
** Busca una macro en la tabla.
*/
int busca_macro(nombre)
  unsigned char *nombre;
{
  int k;

  k = 1;
  while (k < ap_mac) {
    if (astreq(nombre, macs + k, MAX_NOMBRE))
      return k;
    while (macs[k++]);
    k++;
    while (macs[k++]);
  }
  return 0;
}

/*
** Manda un caracter a la salida.
*/
int emite_car(c)
  unsigned char c;
{
  if (c == 0)
    return 0;
  if (salida) {
    if (fputc(c, salida) <= 0) {
      cierra_salida();
      error("Output error");
      cancela();
    }
  } else
    putchar(c);
  return c;
}

/*
** Cambio de linea a la salida.
*/
void emite_nueva_linea()
{
  emite_car('\n');
}

/*
** Ilustra los mensajes de error.
*/
void error(ap)
  unsigned char ap[];
{
  int k;
  unsigned char entrada[81];

  hacia_consola();
  color(11);
  emite_texto("Line ");
  emite_numero(linea_actual);
  emite_texto(", ");
  if (!dentro_funcion)
    emite_car('(');
  if (funcion_actual == NULL)
    emite_texto("from start of the file");
  else
    emite_texto(funcion_actual->nombre);
  if (!dentro_funcion)
    emite_car(')');
  emite_texto(" + ");
  emite_numero(linea_actual - comienzo_funcion);
  emite_texto(": ");
  color(15);
  emite_texto(ap);
  emite_nueva_linea();

  color(14);
  emite_texto(linea);
  emite_nueva_linea();

  k = 0;                /* Busca la posición del error */
  while (k < pos_linea) {
    if (linea[k++] == 9)
      emite_car(9);
    else
      emite_car(' ');
  }
  emite_car('^');
  emite_nueva_linea();
  ++errores;

  hacia_archivo();
  if (pausa) {
    color(10);
    mensaje("Continue (Y/N/Ignore) ? ");
    gets(entrada);
    k = entrada[0];
    if ((k == 'N') || (k == 'n'))
      cancela();
    if ((k == 'I') || (k == 'i'))
      pausa = NO;
  }
}

/*
** Manda una línea a la salida, hace un cambio de linea también.
*/
void emite_linea(ap)
  unsigned char *ap;
{
  emite_texto(ap);
  emite_nueva_linea();
}

/*
** Manda un texto a la salida.
*/
void emite_texto(ap)
  unsigned char *ap;
{
  while (emite_car(*ap++));
}

/*
** Checa si encuentra un operador de expresión.
*/
int encuentra(op)
  unsigned char *op;
{
  int tam_op;

  espacios();
  if(tam_op = streq(linea + pos_linea, op))
    if((*(linea + pos_linea + tam_op) != '=') &&
       (*(linea + pos_linea + tam_op) != *(linea + pos_linea + tam_op - 1)))
      return 1;
  return 0;
}

int streq(cad1, cad2)
  unsigned char cad1[], cad2[];
{
  int k;

  k = 0;
  while (cad2[k]) {
    if ((cad1[k]) != (cad2[k]))
      return 0;
    k++;
  }
  return k;
}

int astreq(cad1, cad2, len)
  unsigned char cad1[], cad2[];
  int len;
{
  int k;

  k = 0;
  while (k < len) {
    if ((cad1[k]) != (cad2[k]))
      break;
    if (cad1[k] == 0)
      break;
    if (cad2[k] == 0)
      break;
    k++;
  }
  if (alfanum(cad1[k]))
    return 0;
  if (alfanum(cad2[k]))
    return 0;
  return k;
}

int match(lit)
  unsigned char *lit;
{
  int k;

  espacios();
  if (k = streq(linea + pos_linea, lit)) {
    pos_linea = pos_linea + k;
    return 1;
  }
  return 0;
}

int amatch(lit, len)
  unsigned char *lit;
  int len;
{
  int k;

  espacios();
  if (k = astreq(linea + pos_linea, lit, len)) {
    pos_linea = pos_linea + k;
    while (alfanum(car_act))
      lee_car();
    return 1;
  }
  return 0;
}

/*
** Salta los espacios en la entrada.
*/
void espacios()
{
  while (1) {
    while (car_act == 0) {
      if (dentro_pp) return;
      preprocesa();
      if (eof)
        break;
    }
    if (isspace(car_act))
      obt_car();
    else
      return;
  }
}

/*
** Compone un entero.
*/
int lee_entero(dir)
  unsigned char *dir;
{
  return *dir | (*(dir+1) << 8) | (*(dir+2) << 16) | (*(dir+3) << 24);
}

/*
** Escribe un entero en una dirección.
*/
void escribe_entero(dir, dato)
  unsigned char *dir;
  int dato;
{
  *dir++ = dato;
  *dir++ = dato >> 8;
  *dir++ = dato >> 16;
  *dir++ = dato >> 24;
}

/*
** Saca un número decimal en la salida.
*/
void emite_numero(numero)
  int numero;
{
  if (numero < 0) {
    emite_car('-');
    if (numero < -9)
      emite_numero(-(numero / 10));
    emite_car(-(numero % 10) + '0');
  } else {
    if (numero > 9)
      emite_numero(numero / 10);
    emite_car((numero % 10) + '0');
  }
}

/*
** Prueba si el caracter dado es una letra.
*/
int letra(c)
  int c;
{
  c = c & 255;
  return (((c >= 'a') && (c <= 'z')) ||
          ((c >= 'A') && (c <= 'Z')) ||
           (c == '_'));
}

/*
** Prueba si el caracter dado es alfanumérico.
*/
int alfanum(c)
  unsigned char c;
{
  return ((letra(c)) || (isdigit(c)));
}

/*
** Evade basura en la entrada.
*/
void basura()
{
  if (alfanum(lee_car()))
    while (alfanum(car_act))
      obt_car();
  else
    while (alfanum(car_act) == 0) {
      if (car_act == 0)
        break;
      obt_car();
    }
  espacios();
}

