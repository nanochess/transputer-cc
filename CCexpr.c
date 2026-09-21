/*
** Compilador de C para transputer.
** Evaluador de Expresiones.
**
** por Oscar Toledo Gutiérrez.
**
** (c) Oscar Toledo G.1995.
**
** Creación: 4 de junio de 1995.
** Revisión: 20 de julio de 1995. Ahora efectua correctamente suma y resta
**                                con apuntadores.
** Revisión: 25 de julio de 1995. Soporte para || y &&.
** Revisión: 25 de julio de 1995. Soporte para el operador trinario ?.
** Revisión: 26 de julio de 1995. Ahora da correctamente el resultado de una
**                                resta de dos apuntadores.
** Revisión: 26 de julio de 1995. Soporte para |= ^= &= += -= /= %= <<= >>=.
** Revisión: 26 de julio de 1995. Nueva función. expr_constante().
** Revisión: 27 de julio de 1995. Soporte para el operador coma.
** Revisión: 29 de julio de 1995. Corrección de los operadores ++ y -- para
**                                variables de tipo char.
** Revisión: 31 de julio de 1995. Corrección de un defecto en el posdecremento.
** Revisión: 10 de agosto de 1995. Soporte para tipos complejos.
** Revisión: 11 de agosto de 1995. Desaparece nivel14(), se combina con
**                                 primaria().
** Revisión: 11 de agosto de 1995. Soporte para short y unsigned short.
** Revisión: 11 de agosto de 1995. Soporte para sizeof.
** Revisión: 11 de agosto de 1995. Soporte para conversiones de tipo.
** Revisión: 12 de agosto de 1995. Corrección de defectos en sizeof y
**                                 conversiones de tipo.
** Revisión: 23 de agosto de 1995. Soporte para los operadores -> y .
** Revisión: 24 de agosto de 1995. Soporte para enumeradores.
** Revisión: 5 de septiembre de 1995. Soporte para asignación, paso cómo
**                                    parametros y resultados de estructuras.
** Revisión: 6 de septiembre de 1995. Nueva función. nivel13dir().
** Revisión: 12 de septiembre de 1995. Soporte para float y double.
** Revisión: 12 de septiembre de 1995. Nuevas funciones. convierte_tipo().
**                                     checa_entero(), checa_numerico(),
**                                     compara_no_cero(), compara_cero(),
**                                     checa_entero_o_apuntador(),
**                                     haz_compatible().
** Revisión: 22 de septiembre de 1995. Corrección de varios errores en
**                                     convierte_tipo y en nivel1().
** Revisión: 27 de septiembre de 1995. Corrección de varios errores.
** Revisión: 22 de noviembre de 1995. Soporte para números de punto flotante.
** Revisión: 26 de noviembre de 1995. Corrección de un defecto en el manejo
**                                    del exponente en números de punto
**                                    flotante.
** Revisión: 26 de noviembre de 1995. Corrección de un defecto en el manejo
**                                    de estructuras.
** Revisión: 30 de noviembre de 1995. Generación de nodos para optimización
**                                    de operaciones con float.
** Revisión: 1o. de diciembre de 1995. Corrección de varios defectos.
** Revisión: 28 de diciembre de 1995. Corrección de un defecto en el manejo
**                                    de unsigned int, no efectuaba bien las
**                                    comparaciones.
** Revisión: 28 de diciembre de 1995. Ahora soporta la letra L en los números.
** Revisión: 28 de diciembre de 1995. Ahora balancea correctamente unsigned.
** Revisión: 28 de diciembre de 1995. Corrección de un defecto en sizeof.
** Revisión: 3 de enero de 1996. Ahora evalua en tiempo de compilación las
**                               expresiones constantes de | & y ^. (¿cómo se
**                               me fue a olvidar?).
** Revisión: 18 de febrero de 1996. Soporte para el operador coma en sentencias
**                                  condicionales. (¿cómo se me fue a olvidar?)
** Revisión: 19 de febrero de 1996. Corrección de defectos en la sintaxis de
**                                  las expresiones.
** Revisión: 9 de marzo de 1996. Genera árboles más óptimos para comparaciones
**                               entre apuntadores.
** Revisión: 8 de abril de 1996. Corrección de un defecto en el procesamiento
**                               de números reales.
** Revisión: 10 de abril de 1996. Corrección de un defecto en la generación de
**                                codigo para el operador trinario.
** Revisión: 15 de abril de 1996. Corrección de un defecto que no permitia el
**                                acceso a estructuras en matrices.
** Revisión: 7 de mayo de 1998. Ahora los árboles de expresiones se crean
**                              dinámicamente.
** Revision: Sep/19/2026. Changed int info[] to unsigned char *info.
*/

/*
** La matriz info contiene información sobre la expresión.
**
** info[0] - Tipo de la expresión hasta el momento.
*/

/*
** Evalua el último arbol generado.
*/
void evalua_arbol(control)
  int control;
{
  es_control = control;
  gen_codigo(ultimo_nodo);
}

/*
** Analiza una expresión, y genera el codigo.
*/
unsigned char *expresion()
{
  struct nodo *origen;
  unsigned char *tipo;

  origen = ultimo_nodo;
  tipo = almacena_expresion(SI);
  evalua_arbol(NO);
  libera_arbol(ultimo_nodo);
  ultimo_nodo = origen;
  return tipo;
}

/*
** Procesa una expresión constante.
*/
int expr_constante()
{
  struct nodo *origen;
  int valor;
  unsigned char *tipo;

  origen = ultimo_nodo;
  checa_entero(almacena_expresion(SI));
  if (ultimo_nodo->oper != N_CONST) {
    libera_arbol(ultimo_nodo);
    ultimo_nodo = origen;
    error("It isn't a constant expression");
    return 1;
  } else {
    valor = ultimo_nodo->esp;
    libera_arbol(ultimo_nodo);
    ultimo_nodo = origen;
    return valor;
  }
}

/*
** Analiza una expresión y la mantiene en memoria.
*/
unsigned char *almacena_expresion(operador_coma)
  int operador_coma;
{
  unsigned char *info;

  if (operador_coma) {
    if (nivel0(&info))
      carga_valor(&info);
  } else {
    if (nivel1(&info))
      carga_valor(&info);
  }
  return info;
}

int nivel0(info)
  unsigned char **info;
{
  int k;
  struct nodo *izq;

  k = nivel1(info);
  while (match(",")) {
    if (k)
      carga_valor(info);
    izq = ultimo_nodo;
    k = nivel1(info);
    crea_nodo(N_COMA, izq, ultimo_nodo, 0);
  }
  return k;
}

int nivel1(info)
  unsigned char **info;
{
  int k, op;
  unsigned char *info2;
  struct nodo *der, *izq;
  unsigned char *tipo, *tipo2;

  k = nivel2(info);
  if (match("="))
    op = N_ASIGNA;
  else if (match("|="))
    op = N_AOR;
  else if (match("^="))
    op = N_AXOR;
  else if (match("&="))
    op = N_AAND;
  else if (match("<<="))
    op = N_ACI;
  else if (match(">>="))
    op = N_ACD;
  else if (match("+="))
    op = N_ASUMA;
  else if (match("-="))
    op = N_ARESTA;
  else if (match("*="))
    op = N_AMUL;
  else if (match("/="))
    op = N_ADIV;
  else if (match("%="))
    op = N_AMOD;
  else return k;
  der = ultimo_nodo;
  tipo = *info;
  if (k == 0)
    if (*tipo != STRUCT)
      req_valorl();
  if (nivel1(&info2)) {
    if (*tipo == STRUCT)
      req_valorl();
    carga_valor(&info2);
    izq = ultimo_nodo;
  } else if (*tipo == STRUCT) {
    tipo2 = info2;
    if (*tipo2 != STRUCT)
      error("struct or union is required");
    else {
      if (lee_entero(tipo + 1) != lee_entero(tipo2 + 1))
        error("Incompatible struct");
    }
    if (op != N_ASIGNA)
      error("Non-compatible assignment");
    crea_nodo(N_COPIA, der, ultimo_nodo, tam_tipo(tipo));
    return 0;
  } else
    izq = ultimo_nodo;
  tipo2 = info2;
  convierte_tipo(&izq, tipo2, tipo);
  if ((*tipo == DOUBLE || *tipo == FLOAT)
  && (op == N_AOR || op == N_AXOR
   || op == N_AAND || op == N_ACI
   || op == N_ACD || op == N_AMOD))
    error("Operation cannot be done with double");
  if ((op == N_ASUMA) || (op == N_ARESTA)) {
    if (k = dobla(tipo, izq)) {
      if (k == 2) {
        if (multi == 4) {
          if (op == N_ARESTA) {
            crea_nodo(N_NEG, izq, NULL, 0);
            izq = ultimo_nodo;
          }
          op = N_AIXP;
        } else if (multi != 1) {
          crea_nodo(N_CONST, NULL, NULL, multi);
          crea_nodo(N_MUL, izq, ultimo_nodo, 0);
          izq = ultimo_nodo;
        }
      }
    }
  }
  if ((op == N_ARESTA) && (izq->oper == N_CONST)) {
    op = N_ASUMA;
    izq->esp = -izq->esp;
  }
  if (*tipo == APUNTADOR)
    tipo = t_int;
  crea_nodo(op, izq, der, *tipo);
  return 0;
}

int nivel2(info)
  unsigned char **info;
{
  int k;
  unsigned char *info2;
  unsigned char *info3;
  struct nodo *ext, *izq, *der;

  k = nivel3(info);
  espacios();
  if (car_act != '?')
    return k;
  if (k)
    carga_valor(info);
  ext = ultimo_nodo;
  pos_linea++;
  if (nivel0(&info2))
    carga_valor(&info2);
  izq = ultimo_nodo;
  pide(":");
  if (nivel2(&info3))
    carga_valor(&info3);
  der = ultimo_nodo;
  if ((ext->oper == N_CONST) &&
     (izq->oper == N_CONST) &&
     (der->oper == N_CONST)) {
    if (ext->esp) ultimo_nodo->esp = izq->esp;
    libera_arbol(ext);
    libera_arbol(izq);
  } else {
    haz_compatible(&izq, &info2, &der, &info3);
    crea_nodo(N_TRI, izq, der, 0);
    ultimo_nodo->tri = ext;
  }
  *info = info2;
  return 0;
}

int nivel3(info)
  unsigned char **info;
{
  int k;
  unsigned char *info2;
  struct nodo *izq;

  k = nivel4(info);
  espacios();
  if (streq(linea + pos_linea, "||") == 0)
    return k;
  if (k)
    carga_valor(info);
  while (match("||")) {
    checa_numerico(*info);
    compara_no_cero(*info);
    izq = ultimo_nodo;
    if (nivel4(&info2))
      carga_valor(&info2);
    checa_numerico(info2);
    compara_no_cero(info2);
    crea_nodo(N_ORB, izq, ultimo_nodo, 0);
  }
  return 0;
}

int nivel4(info)
  unsigned char **info;
{
  int k;
  unsigned char *info2;
  struct nodo *izq;

  k = nivel5(info);
  espacios();
  if (streq(linea + pos_linea, "&&") == 0)
    return k;
  if (k)
    carga_valor(info);
  while (match("&&")) {
    checa_numerico(*info);
    compara_no_cero(*info);
    izq = ultimo_nodo;
    if (nivel5(&info2))
      carga_valor(&info2);
    checa_numerico(info2);
    compara_no_cero(info2);
    crea_nodo(N_ANDB, izq, ultimo_nodo, 0);
  }
  return 0;
}

int nivel5(info)
  unsigned char **info;
{
  int k;
  unsigned char *info2;
  struct nodo *izq;

  k = nivel6(info);
  if (encuentra("|") == 0) return k;
  if (k)
    carga_valor(info);
  while (encuentra("|")) {
    pos_linea++;
    izq = ultimo_nodo;
    checa_entero(*info);
    if (nivel6(&info2))
      carga_valor(&info2);
    checa_entero(info2);
    if (izq->oper == N_CONST && ultimo_nodo->oper == N_CONST) {
      ultimo_nodo->esp |= izq->esp;
      libera_arbol(izq);
    } else
      crea_nodo(N_OR, izq, ultimo_nodo, 0);
  }
  return 0;
}

int nivel6(info)
  unsigned char **info;
{
  int k;
  unsigned char *info2;
  struct nodo *izq;

  k = nivel7(info);
  if (encuentra("^") == 0) return k;
  if (k)
    carga_valor(info);
  while (encuentra("^")) {
    pos_linea++;
    izq = ultimo_nodo;
    checa_entero(*info);
    if (nivel7(&info2))
      carga_valor(&info2);
    checa_entero(info2);
    if (izq->oper == N_CONST && ultimo_nodo->oper == N_CONST) {
      ultimo_nodo->esp ^= izq->esp;
      libera_arbol(izq);
    } else
      crea_nodo(N_XOR, izq, ultimo_nodo, 0);
  }
  return 0;
}

int nivel7(info)
  unsigned char **info;
{
  int k;
  unsigned char *info2;
  struct nodo *izq;

  k = nivel8(info);
  if (encuentra("&") == 0) return k;
  if (k)
    carga_valor(info);
  while (encuentra("&")) {
    pos_linea++;
    izq = ultimo_nodo;
    checa_entero(*info);
    if (nivel8(&info2))
      carga_valor(&info2);
    checa_entero(info2);
    if (izq->oper == N_CONST && ultimo_nodo->oper == N_CONST) {
      ultimo_nodo->esp &= izq->esp;
      libera_arbol(izq);
    } else
      crea_nodo(N_AND, izq, ultimo_nodo, 0);
  }
  return 0;
}

int nivel8(info)
  unsigned char **info;
{
  int k;
  unsigned char *info2;
  struct nodo *izq, *der;

  k = nivel9(info);
  if ((encuentra("==") == 0) &&
      (encuentra("!=") == 0))
    return k;
  if (k)
    carga_valor(info);
  while (1) {
    izq = ultimo_nodo;
    if (encuentra("==")) {
      pos_linea += 2;
      if (nivel9(&info2))
        carga_valor(&info2);
      der = ultimo_nodo;
      if (haz_compatible(&izq, info, &der, &info2))
        crea_nodo(N_IGUALPF, izq, der, (**info == FLOAT));
      else if (der->oper == N_CONST) {
        if (izq->oper == N_CONST) {
          crea_nodo(N_CONST, NULL, NULL, izq->esp == der->esp);
          libera_arbol(izq);
        } else
          crea_nodo(N_CIGUAL, izq, NULL, der->esp);
        libera_arbol(der);
      } else if (izq->oper == N_CONST) {
        crea_nodo(N_CIGUAL, der, 0, izq->esp);
        libera_arbol(izq);
      } else
        crea_nodo(N_IGUAL, izq, der, 0);
    } else if (encuentra("!=")) {
      pos_linea += 2;
      if (nivel9(&info2))
        carga_valor(&info2);
      der = ultimo_nodo;
      if (haz_compatible(&izq, info, &der, &info2)) {
        crea_nodo(N_IGUALPF, izq, der, (**info == FLOAT));
        crea_nodo(N_NOT, ultimo_nodo, NULL, 0);
      } else if (der->oper == N_CONST) {
        if (izq->oper == N_CONST) {
          crea_nodo(N_CONST, NULL, NULL, izq->esp != der->esp);
          libera_arbol(izq);
        } else {
          crea_nodo(N_CIGUAL, izq, NULL, der->esp);
          crea_nodo(N_NOT, ultimo_nodo, NULL, 0);
        }
        libera_arbol(der);
      } else if (izq->oper == N_CONST) {
        crea_nodo(N_CIGUAL, der, NULL, izq->esp);
        crea_nodo(N_NOT, ultimo_nodo, NULL, 0);
        libera_arbol(izq);
      } else {
        crea_nodo(N_IGUAL, izq, der, 0);
        crea_nodo(N_NOT, ultimo_nodo, NULL, 0);
      }
    } else
      return 0;
    *info = t_int;
  }
}

int nivel9(info)
  unsigned char **info;
{
  int k;

  k = nivel10(info);
  if ((encuentra("<") == 0) &&
      (encuentra(">") == 0) &&
      (encuentra("<=") == 0) &&
      (encuentra(">=") == 0))
    return k;
  if (k)
    carga_valor(info);
  while (1) {
    if (encuentra("<=")) {
      pos_linea = pos_linea + 2;
      nivel9eval(1, info);
    } else if (encuentra(">=")) {
      pos_linea = pos_linea + 2;
      nivel9eval(2, info);
    } else if (encuentra("<")) {
      pos_linea++;
      nivel9eval(3, info);
    } else if (encuentra(">")) {
      pos_linea++;
      nivel9eval(4, info);
    } else
      return 0;
  }
}

void nivel9eval(k, info)
  int k;
  unsigned char **info;
{
  unsigned char *info2;
  struct nodo *izq, *der;
  unsigned char *tipo;

  izq = ultimo_nodo;
  if (nivel10(&info2))
    carga_valor(&info2);
  der = ultimo_nodo;
  if (haz_compatible(&izq, info, &der, &info2)) {
    if (k == 4 || k == 1)
      crea_nodo(N_MAYORPF, izq, der, (**info == FLOAT));
    else
      crea_nodo(N_MAYORPF, der, izq, (**info == FLOAT));
    if (k == 1 || k == 2)
      crea_nodo(N_NOT, ultimo_nodo, NULL, 0);
    info[0] = t_int;
    return;
  }
  tipo = *info;
  if (*tipo == UINT) {
    nivel9op(izq, k);
    *info = t_int;
    return;
  }
  tipo = info2;
  if (*tipo == UINT) {
    nivel9op(izq, k);
    *info = t_int;
    return;
  }
  if (k == 4) {
    if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
      crea_nodo(N_CONST, NULL, NULL, izq->esp > der->esp);
      libera_arbol(izq);
      libera_arbol(der);
    } else
      crea_nodo(N_MAYOR, izq, der, 0);
  } else if (k == 3) {
    if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
      crea_nodo(N_CONST, NULL, NULL, izq->esp < der->esp);
      libera_arbol(izq);
      libera_arbol(der);
    } else
      crea_nodo(N_MAYOR, der, izq, 0);
  } else if (k == 1) {
    if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
      crea_nodo(N_CONST, NULL, NULL, izq->esp <= der->esp);
      libera_arbol(izq);
      libera_arbol(der);
    } else {
      crea_nodo(N_MAYOR, izq, der, 0);
      crea_nodo(N_NOT, ultimo_nodo, 0);
    }
  } else {
    if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
      crea_nodo(N_CONST, NULL, NULL, izq->esp >= der->esp);
      libera_arbol(izq);
      libera_arbol(der);
    } else {
      crea_nodo(N_MAYOR, der, izq, 0);
      crea_nodo(N_NOT, ultimo_nodo, 0);
    }
  }
  *info = t_int;
}

void nivel9op(izq, k)
  int izq, k;
{
  if (k == 4)
    crea_nodo(N_SMAYOR, izq, ultimo_nodo, 0);
  else if (k == 3)
    crea_nodo(N_SMAYOR, ultimo_nodo, izq, 0);
  else if (k == 1) {
    crea_nodo(N_SMAYOR, izq, ultimo_nodo, 0);
    crea_nodo(N_NOT, ultimo_nodo, NULL, 0);
  } else {
    crea_nodo(N_SMAYOR, ultimo_nodo, izq, 0);
    crea_nodo(N_NOT, ultimo_nodo, NULL, 0);
  }
}

int nivel10(info)
  unsigned char **info;
{
  int k;
  unsigned char *info2;
  struct nodo *izq;

  k = nivel11(info);
  if ((encuentra(">>") == 0) &&
      (encuentra("<<") == 0))
    return k;
  if (k)
    carga_valor(info);
  while (1) {
    izq = ultimo_nodo;
    if (encuentra(">>")) {
      pos_linea += 2;
      checa_entero(*info);
      if (nivel11(&info2))
        carga_valor(&info2);
      checa_entero(info2);
      if ((izq->oper == N_CONST) && (ultimo_nodo->oper == N_CONST)) {
        ultimo_nodo->esp = izq->esp >> ultimo_nodo->esp;
        libera_arbol(izq);
      } else
        crea_nodo(N_CD, izq, ultimo_nodo, 0);
    } else if (encuentra("<<")) {
      pos_linea += 2;
      checa_entero(*info);
      if (nivel11(&info2))
        carga_valor(&info2);
      checa_entero(info2);
      if ((izq->oper == N_CONST) && (ultimo_nodo->oper == N_CONST)) {
        ultimo_nodo->esp = izq->esp << ultimo_nodo->esp;
        libera_arbol(izq);
      } else
        crea_nodo(N_CI, izq, ultimo_nodo, 0);
    } else
      return 0;
  }
}

int nivel11(info)
  unsigned char **info;
{
  int k, tam;
  unsigned char *info2;
  struct nodo *izq, *der;
  unsigned char *tipo;

  k = nivel12(info);
  if ((encuentra("+") == 0) &&
      (encuentra("-") == 0))
    return k;
  if (k)
    carga_valor(info);
  while (1) {
    izq = ultimo_nodo;
    if (encuentra("+")) {
      pos_linea++;
      if (nivel12(&info2))
        carga_valor(&info2);
      der = ultimo_nodo;
      if (haz_compatible(&izq, info, &der, &info2)) {
        crea_nodo(N_SUMAPF, izq, der, (**info == FLOAT));
      } else {
        if (k = dobla(*info, der)) {
          if (k == 2 && multi == 4) {
            crea_nodo(N_IXP, der, izq, 0);
            continue;
          } else if (k == 2 && multi != 1) {
            crea_nodo(N_CONST, NULL, NULL, multi);
            crea_nodo(N_MUL, der, ultimo_nodo, 0);
            der = ultimo_nodo;
          }
        } else {
          *info = info2;
          if (k = dobla(*info, izq)) {
            if (k == 2 && multi == 4) {
              crea_nodo(N_IXP, izq, der, 0);
              continue;
            } else if (k == 2 && multi != 1) {
              crea_nodo(N_CONST, NULL, NULL, multi);
              crea_nodo(N_MUL, izq, ultimo_nodo, 0);
              izq = ultimo_nodo;
            }
          }
        }
        if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
          crea_nodo(N_CONST, NULL, NULL, izq->esp + der->esp);
          libera_arbol(der);
          libera_arbol(izq);
        } else if (der->oper == N_CONST) {
          crea_nodo(N_CSUMA, izq, NULL, der->esp);
          libera_arbol(der);
        } else if (izq->oper == N_CONST) {
          crea_nodo(N_CSUMA, der, NULL, izq->esp);
          libera_arbol(izq);
        } else
          crea_nodo(N_SUMA, izq, der, 0);
      }
    } else if (encuentra("-")) {
      pos_linea++;
      if (nivel12(&info2))
        carga_valor(&info2);
      der = ultimo_nodo;
      if (haz_compatible(&izq, info, &der, &info2)) {
        crea_nodo(N_RESTAPF, izq, der, (**info == FLOAT));
      } else {
        tipo = *info;
        if (*tipo == APUNTADOR || *tipo == MATRIZ) {
          tipo = info2;
          if (*tipo == APUNTADOR || *tipo == MATRIZ) {
            crea_nodo(N_RESTA, izq, der, 0);
            if (*tipo == APUNTADOR)
              tam = tam_tipo(tipo + 1);
            else
              tam = tam_tipo(tipo + 5);
            if (tam == 4)
              crea_nodo(N_CUENTA, ultimo_nodo, NULL, 0);
            else if (tam != 1) {
              izq = ultimo_nodo;
              crea_nodo(N_CONST, NULL, NULL, tam);
              crea_nodo(N_DIV, izq, ultimo_nodo, 0);
            }
            *info = t_int;
            continue;
          }
        }
        if (k = dobla(*info, der)) {
          if (k == 2 && multi == 4) {
            crea_nodo(N_NEG, der, NULL, 0);
            crea_nodo(N_IXP, ultimo_nodo, izq, 0);
            continue;
          } else if (k == 2 && multi != 1) {
            crea_nodo(N_CONST, NULL, NULL, multi);
            crea_nodo(N_MUL, der, ultimo_nodo, 0);
            der = ultimo_nodo;
          }
        }
        if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
          crea_nodo(N_CONST, NULL, NULL, izq->esp - der->esp);
          libera_arbol(izq);
          libera_arbol(der);
        } else if (der->oper == N_CONST) {
          crea_nodo(N_CSUMA, izq, NULL, -der->esp);
          libera_arbol(der);
        } else
          crea_nodo(N_RESTA, izq, der, 0);
      }
    } else
      return 0;
  }
}

int nivel12(info)
  unsigned char **info;
{
  int k;
  unsigned char *info2;
  struct nodo *izq, *der;

  k = nivel13(info);
  if ((encuentra("*") == 0) &&
      (encuentra("/") == 0) &&
      (encuentra("%") == 0))
    return k;
  if (k)
    carga_valor(info);
  while (1) {
    izq = ultimo_nodo;
    if (encuentra("*")) {
      pos_linea++;
      if (nivel13(&info2))
        carga_valor(&info2);
      der = ultimo_nodo;
      if (haz_compatible(&izq, info, &der, &info2)) {
        crea_nodo(N_MULPF, izq, der, (**info == FLOAT));
      } else {
        checa_entero(*info);
        checa_entero(info2);
        if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
          crea_nodo(N_CONST, NULL, NULL, izq->esp * der->esp);
          libera_arbol(izq);
          libera_arbol(der);
        } else
          crea_nodo(N_MUL, izq, der, 0);
      }
    } else if (encuentra("/")) {
      pos_linea++;
      if (nivel13(&info2))
        carga_valor(&info2);
      der = ultimo_nodo;
      if (haz_compatible(&izq, info, &der, &info2)) {
        crea_nodo(N_DIVPF, izq, der, (**info == FLOAT));
      } else {
        checa_entero(*info);
        checa_entero(info2);
        if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
          crea_nodo(N_CONST, NULL, NULL, izq->esp / der->esp);
          libera_arbol(izq);
          libera_arbol(der);
        } else
          crea_nodo(N_DIV, izq, der, 0);
      }
    } else if (encuentra("%")) {
      pos_linea++;
      checa_entero(*info);
      if (nivel13(&info2))
        carga_valor(&info2);
      checa_entero(info2);
      der = ultimo_nodo;
      if ((izq->oper == N_CONST) && (der->oper == N_CONST)) {
        crea_nodo(N_CONST, NULL, NULL, izq->esp % der->esp);
        libera_arbol(izq);
        libera_arbol(der);
      } else
        crea_nodo(N_MOD, izq, der, 0);
    } else
      return 0;
  }
}

int nivel13(info)
  unsigned char **info;
{
  int k, p;
  unsigned char *tipo, *tipo2;
  struct nodo *n;

  if (match("++")) {
    if (nivel13(info) == 0)
      req_valorl();
    checa_entero_o_apuntador(*info);
    nivel13inc(info);
    return 0;
  } else if (match("--")) {
    if (nivel13(info) == 0)
      req_valorl();
    checa_entero_o_apuntador(*info);
    nivel13dec(info);
    return 0;
  } else if (match("-")) {
    if (nivel13(info))
      carga_valor(info);
    checa_numerico(*info);
    tipo = *info;
    if (ultimo_nodo->oper == N_CONST)
      ultimo_nodo->esp = -ultimo_nodo->esp;
    else if (*tipo == DOUBLE || *tipo == FLOAT) {
      n = ultimo_nodo;
      if (*tipo == DOUBLE)
        crea_nodo(N_CEROPF, NULL, NULL, 0);
      else
        crea_nodo(N_CEROF, NULL, NULL, 0);
      crea_nodo(N_RESTAPF, ultimo_nodo, n, 0);
    } else
      crea_nodo(N_NEG, ultimo_nodo, NULL, 0);
    return 0;
  } else if (match("~")) {
    if (nivel13(info))
      carga_valor(info);
    checa_entero(*info);
    if (ultimo_nodo->oper == N_CONST)
      ultimo_nodo->esp = ~ultimo_nodo->esp;
    else
      crea_nodo(N_COM, ultimo_nodo, NULL, 0);
    return 0;
  } else if (match("!")) {
    if (nivel13(info))
      carga_valor(info);
    checa_numerico(*info);
    tipo = *info;
    if (ultimo_nodo->oper == N_CONST)
      ultimo_nodo->esp = !ultimo_nodo->esp;
    else if (*tipo == DOUBLE || *tipo == FLOAT)
      compara_cero(tipo);
    else
      crea_nodo(N_NOT, ultimo_nodo, NULL, 0);
    *info = t_int;
    return 0;
  } else if (match("*")) {
    return nivel13ap(info);
  } else if (match("&")) {
    return nivel13dir(info);
  } else if (amatch("sizeof", 6)) {
    if (match("(")) {
      if (p_tipo_1(NO)) {
        p_tipo_2(NULL);
        pide(")");
        crea_nodo(N_CONST, NULL, NULL, tam_tipo(tipo_proc));
        *info = t_int;
        return 0;
      } else
        k = primaria(info, SI);
    } else
      k = nivel13(info);
    crea_nodo(N_CONST, NULL, NULL, tam_tipo(*info));
    *info = t_int;
    return 0;
  } else if (match("(")) {
    if (p_tipo_1(NO)) {
      p_tipo_2(NULL);
      pide(")");
      tipo = tipo_proc;
      if (nivel13(info))
        carga_valor(info);
      tipo2 = *info;
      *info = tipo;
      n = ultimo_nodo;
      convierte_tipo(&n, tipo2, tipo);
      return 0;
    } else k = primaria(info, SI);
  } else k = primaria(info, NO);
  if (match("++")) {
    if (k == 0)
      req_valorl();
    checa_entero_o_apuntador(*info);
    nivel13pinc(info);
    return 0;
  }
  if (match("--")) {
    if (k == 0)
      req_valorl();
    checa_entero_o_apuntador(*info);
    nivel13pdec(info);
    return 0;
  }
  return k;
}

int nivel13ap(info)
  unsigned char **info;
{
  int k;
  unsigned char *ap;

  k = nivel13(info);
  if (k)
    carga_valor(info);
  ap = *info;
  if (*ap == APUNTADOR)
    ap++;
  else if (*ap == MATRIZ)
    ap += 5;
  else
    error("It isn't a pointer or array");
  *info = ap;
  if (*ap == FUNCION || *ap == MATRIZ || *ap == STRUCT)
    return 0;
  else
    return 1;
}

int nivel13dir(info)
  unsigned char **info;
{
  unsigned char *nuevo_tipo, *tipo;

  if (nivel13(info) == 0) {
    tipo = *info;
    if (*tipo != STRUCT)
      error("Illegal address");
  }
  nuevo_tipo = sig_tipo;
  guarda_tipo(APUNTADOR);
  copia_tipo(*info);
  *info = nuevo_tipo;
  return 0;
}

void nivel13inc(info)
  unsigned char **info;
{
  unsigned char *tipo;
  int inc;

  tipo = *info;
  if (*tipo == APUNTADOR) {
    inc = tam_tipo(tipo + 1);
    tipo = t_int;
  } else
    inc = 1;
  crea_nodo(N_INC, ultimo_nodo, NULL, *tipo);
  ultimo_nodo->extra_val = inc;
}

void nivel13dec(info)
  unsigned char **info;
{
  nivel13inc(info);
  ultimo_nodo->extra_val = -ultimo_nodo->extra_val;
}

void nivel13pinc(info)
  unsigned char **info;
{
  nivel13inc(info);
  ultimo_nodo->oper = N_PINC;
}

void nivel13pdec(info)
  unsigned char **info;
{
  nivel13dec(info);
  ultimo_nodo->oper = N_PINC;
}

int primaria(info, sin_parentesis)
  unsigned char **info;
  int sin_parentesis;
{
  unsigned char *ap, nombre[TAM_NOMBRE];
  int k, etiq, tam, punto;
  struct nodo *izq, *der;
  unsigned char *tipo;
  unsigned char *info2;
  struct nombres *ap1;
  struct enumerador *ap2;
  struct rotulo *ap3;
  struct miembro *ap4;

  if (sin_parentesis || match("(")) {
    k = nivel0(info);
    pide(")");
  } else if (nombre_legal(nombre)) {
    if ((ap1 = busca_loc(nombre))
     || (ap1 = busca_glb(nombre))) {
      if (ap1->ident == TYPEDEF
       || ap1->ident == ETIQUETA)
        error("It isn't a variable or function");
      if (ap1->ident != FUNCION) {
        if (ap1->clase == AUTO)
          dir_var_loc(ap1);
        else
          dir_var_glb(ap1);
      } else
        dir_func(ap1);
      *info = tipo = ap1->tipo;
      if (*tipo == MATRIZ || *tipo == FUNCION || *tipo == STRUCT)
        k = 0;
      else
        k = 1;
    } else if ((ap2 = busca_enum(nombre)) != NULL) {
      crea_nodo(N_CONST, NULL, NULL, ap2->valor);
      *info = t_int;
      k = 0;
    } else {
      dir_func(nueva_glb(nombre, FUNCION, STATIC, t_func, FUNC_REF));
      *info = t_func;
      k = 0;
    }
  } else if (constante(info)) {
    k = 0;
  } else {
    error("Invalid expression");
    crea_nodo(N_CONST, NULL, NULL, 0);
    basura();
    *info = t_int;
    k = 0;
  }
  tipo = *info;
  while (1) {
    if (match("[")) {
      if (*tipo != APUNTADOR && *tipo != MATRIZ) {
        error("Index cannot be used");
        basura();
        pide("]");
        return 0;
      }
      if (k)
        carga_valor(info);
      izq = ultimo_nodo;
      if (nivel1(&info2))
        carga_valor(&info2);
      pide("]");
      if (*tipo == APUNTADOR)
        tam = tam_tipo(++tipo);
      else
        tam = tam_tipo(tipo += 5);
      if (tam == 8) {
        if (ultimo_nodo->oper == N_CONST) {
          der = ultimo_nodo;
          crea_nodo(N_LDNLP, izq, NULL, der->esp * 2);
          libera_arbol(der);
        } else
          crea_nodo(N_IXF, ultimo_nodo, izq, 0);
      } else if (tam == 4) {
        if (ultimo_nodo->oper == N_CONST) {
          der = ultimo_nodo;
          crea_nodo(N_LDNLP, izq, NULL, der->esp);
          libera_arbol(der);
        } else
          crea_nodo(N_IXP, ultimo_nodo, izq, 0);
      } else if (tam == 1) {
        if (ultimo_nodo->oper == N_CONST) {
          der = ultimo_nodo;
          crea_nodo(N_CSUMA, izq, NULL, der->esp);
          libera_arbol(der);
        } else
          crea_nodo(N_SUMA, ultimo_nodo, izq, 0);
      } else {
        if (ultimo_nodo->oper == N_CONST) {
          der = ultimo_nodo;
          crea_nodo(N_CSUMA, izq, NULL, der->esp * tam);
          libera_arbol(der);
        } else {
          der = ultimo_nodo;
          crea_nodo(N_CONST, NULL, NULL, tam);
          crea_nodo(N_MUL, der, ultimo_nodo, 0);
          crea_nodo(N_SUMA, izq, ultimo_nodo, 0);
        }
      }
      *info = tipo;
      if (*tipo == MATRIZ || *tipo == STRUCT)
        k = 0;
      else
        k = 1;
    } else if (match("(")) {
      if (k)
        carga_valor(info);
      if (*tipo != FUNCION)
        error("Type isn't function");
      else
        ++tipo;
      if (ultimo_nodo->oper == N_APFUNC)
        llama_funcion((struct nombres *) ultimo_nodo->tri, tipo);
      else
        llama_funcion(NULL, tipo);
      *info = tipo;
      k = 0;
    } else {
      if (match("."))
        punto = 1;
      else if (match("->"))
        punto = 2;
      else
        punto = 0;
      if (punto) {
        if (nombre_legal(nombre) == 0)
          error("Illegal name for field");
        if (punto == 2) {
          if (k)
            carga_valor(info);
          if (*tipo == APUNTADOR)
            tipo++;
          else if (*tipo == MATRIZ)
            tipo += 5;
          else
            error("It isn't a pointer or array");
          *info = tipo;
        }
        if (*tipo != STRUCT) {
          error("It isn't a struct or union");
          continue;
        }
        tam = lee_entero(tipo + 1);
        ap3 = lista_estructura;
        while (tam) {
          tam--;
          ap3 = ap3->sig;
        }
        if (ap3->tam == 0) {
          error("Incomplete struct or union");
          continue;
        }
        ap4 = ap3->lista;
        while (ap4 != NULL) {
          if (astreq(nombre, ap4->nombre, MAX_NOMBRE)) {
            *info = tipo = ap4->tipo;
            if (*tipo == FUNCION || *tipo == MATRIZ || *tipo == STRUCT)
              k = 0;
            else
              k = 1;
            crea_nodo(N_CSUMA, ultimo_nodo, NULL, ap4->posicion);
            break;
          }
          ap4 = ap4->sig;
        }
        if (ap4 == NULL)
          error("Undefined field");
      } else
        break;
    }
  }
  return k;
}

void req_valorl()
{
  error("It must be a l-value");
}

/*
** Compila una llamada a una función
**
** Invocada por "primaria", esta función llamará a la función
** nombrada o a una función indirecta.
*/
void llama_funcion(ap, tipo_funcion)
  struct nombres *ap;
  unsigned char *tipo_funcion;
{
  unsigned char *info;
  int tam;
  struct nodo *izq, *anterior, *primero;
  unsigned char *tipo;

  anterior = primero = NULL;
  espacios();                /* Ya ha sido tomado el parentesis inicial */
  if (ap == NULL)
    izq = ultimo_nodo;       /* Llamada indirecta */
  if (*tipo_funcion == STRUCT) {
    tam = (tam_tipo(tipo_funcion) + 3) / 4;
    crea_nodo(N_RESULTA, ultimo_nodo, NULL, tam);
    primero = anterior = ultimo_nodo;
  }
  while (car_act != ')') {
    if (fin_sentencia())
      break;
    tam = 0;
    if (nivel1(&info))
      carga_valor(&info);     /* Obtiene un argumento */
    else {
      tipo = info;
      if (*tipo == STRUCT)
        tam = (tam_tipo(tipo) + 3) / 4;
    }
    tipo = info;
    if (*tipo == DOUBLE)
      crea_nodo(N_PARF, ultimo_nodo, NULL, tam);
    else if (*tipo == FLOAT) {
      crea_nodo(N_CONVFD, ultimo_nodo, NULL, 0);
      crea_nodo(N_PARF, ultimo_nodo, NULL, tam);
    } else
      crea_nodo(N_PAR, ultimo_nodo, NULL, tam);
    if (primero == NULL)
      primero = ultimo_nodo;
    if (anterior != NULL)
      anterior->tri = ultimo_nodo;
    anterior = ultimo_nodo;
    if (match(",") == 0)
      break;
    espacios();
  }
  pide(")");
  if (ap == NULL) {
    crea_nodo(N_FUNCI, primero, NULL, 0);
    ultimo_nodo->tri = izq;
  } else {
    crea_nodo(N_FUNC, primero, NULL, 0);
    ultimo_nodo->tri = (struct nodo *) ap->nombre;
  }
}

/*
** Carga el valor de una dirección de memoria.
*/
void carga_valor(info)
  unsigned char **info;
{
  unsigned char *tipo;
    
  tipo = *info;
  if (*tipo == CHAR) {
    crea_nodo(N_CBYTE, ultimo_nodo, NULL, 0);
  } else if (*tipo == SHORT) {
    crea_nodo(N_CSHORT, ultimo_nodo, NULL, 0);
  } else if (*tipo == USHORT) {
    crea_nodo(N_CUSHORT, ultimo_nodo, NULL, 0);
  } else if (*tipo == FLOAT)
    crea_nodo(N_CFLOAT, ultimo_nodo, NULL, 0);
  else if (*tipo == DOUBLE)
    crea_nodo(N_CDOUBLE, ultimo_nodo, NULL, 0);
  else if (*tipo == VOID)
    error("It has void type");
  else
    crea_nodo(N_CPAL, ultimo_nodo, NULL, 0);
}

/*
** Carga la dirección de una variable local
*/
void dir_var_loc(var)
  struct nombres *var;
{
  crea_nodo(N_LDLP, NULL, NULL, var->posicion);
}

/*
** Carga la dir. de una variable global.
*/
void dir_var_glb(var)
  struct nombres *var;
{
  int j;

  j = var->posicion;
  enlace();
  crea_nodo(N_LDNLP, ultimo_nodo, NULL, j);
}

/*
** Carga la dir. de una función.
*/
void dir_func(ap)
  struct nombres *ap;
{
  crea_nodo(N_APFUNC, NULL, NULL, 0);
  ultimo_nodo->tri = (struct nodo *) ap->nombre;
}

/*
** Carga la dir. de comienzo de las variables estáticas.
*/
void enlace()
{
  crea_nodo(N_LDL, NULL, NULL, 1);
}

/*
** Checa si es necesario doblar para suma o resta con apuntadores.
*/
int dobla(tipo, nodo)
  unsigned char *tipo;
  struct nodo *nodo;
{
  int cuanto;

  if ((*tipo != APUNTADOR) && (*tipo != MATRIZ))
    return 0;                         /* no es necesario */
  if (*tipo == APUNTADOR)
    cuanto = tam_tipo(tipo + 1);
  else
    cuanto = tam_tipo(tipo + 5);
  if (nodo->oper == N_CONST) {
    nodo->esp *= cuanto;              /* es una constante */
    return 1;
  }
  multi = cuanto;
  return 2;    /* optimizar segun sea suma o resta */
}

/*
** Prueba si la expresión es cero y salta.
*/
void prueba(etiq, parentesis)
  int etiq, parentesis;
{
  int info[1];
  struct nodo *origen;
  int etiq2;
  unsigned char *tipo;

  origen = ultimo_nodo;
  usa_expr = SI;
  if (parentesis) {
    pide("(");
    compara_no_cero(almacena_expresion(SI));
    pide(")");
  } else
    compara_no_cero(almacena_expresion(SI));
  if (ultimo_nodo->oper == N_CONST) {
    if (ultimo_nodo->esp == 0)
      salto(etiq);
  } else {
    if (ultimo_nodo->oper == N_NOT)
      if (ultimo_nodo->izq->oper == N_NOT)
        ultimo_nodo = ultimo_nodo->izq->izq;
    if ((ultimo_nodo->oper == N_ANDB) ||
        (ultimo_nodo->oper == N_ORB)) {
      etiq_and = etiq;
      etiq_or = nueva_etiq;
      evalua_arbol(SI);
      salta_si_falso(etiq);
      emite_etiq(etiq_or);
      dos_puntos();
      emite_nueva_linea();
    } else {
      evalua_arbol(SI);
      salta_si_falso(etiq);
    }
  }
  libera_arbol(ultimo_nodo);
  ultimo_nodo = origen;
}

int constante(info)
  unsigned char **info;
{
  int val[1], queonda;

  if (queonda = numero_real(val)) {
    if (queonda == 2)
      crea_nodo(N_CEROPF, NULL, NULL, 0);
    else
      crea_nodo(N_NUMPF, NULL, NULL, val[0]);
    *info = t_double;
  } else if (numero(val)) {
    crea_nodo(N_CONST, NULL, NULL, val[0]);
    *info = t_int;
  } else if (cad_caracteres(val)) {
    crea_nodo(N_CONST, NULL, NULL, val[0]);
    *info = t_int;
  } else if (cad_literal(val)) {
    crea_nodo(N_LIT, NULL, NULL, val[0]);
    *info = t_achar;
  } else
    return 0;
  return 1;
}

int numero_real(val)
  int val[];
{
  double num, escala;
  unsigned char *comienzo, *codigo;
  int k, menos;

  comienzo = codigo = linea + pos_linea;
  k = menos = 1;
  while (k) {
    k = 0;
    if (*codigo == '+') {
      ++codigo;
      k = 1;
    }
    if (*codigo == '-') {
      ++codigo;
      k = 1;
      menos = -menos;
    }
  }
  while (isdigit(*codigo))
    ++codigo;
  if (*codigo != '.' && toupper(*codigo) != 'E')
    return 0;
  num = 0;
  if (*codigo == '.') {
    ++codigo;
    while (isdigit(*codigo))
      ++codigo;
    pos_linea = codigo - linea;
    while (*--codigo != '.')
      num = (num + (*codigo - '0')) / 10;
  } else
    pos_linea = codigo - linea;
  escala = 1;
  while (--codigo >= comienzo) {
    num += escala * (*codigo - '0');
    escala *= 10;
  }
  if (toupper(car_act) == 'E') {
    int neg, exp;

    obt_car();
    if (numero(&exp) == 0) {
      error("Wrong exponent");
      exp = 0;
    }
    if (exp < 0) {
      neg = 1;
      exp = -exp;
    } else
      neg = 0;
    k = 32;
    escala = 1;
    while (k) {
      escala *= escala;
      if (k & exp)
        escala *= 10;
      k >>= 1;
    }
    if (neg) num /= escala;
    else     num *= escala;
  }
  if (menos < 0)
    num = -num;
  if (num == 0)
    return 2;
  for (k = 0; k < const_definidas; ++k) {
    if (constantes[k].valor == num)
      break;
  }
  val[0] = k;
  if (k == const_definidas)
    if (const_definidas == MAX_CONST)
      error("Too many floating-point constants");
    else
      constantes[const_definidas++].valor = num;
  return 1;
}

int numero(val)
  int val[];
{
  int k, menos;
  unsigned char c;

  k = menos = 1;
  while (k) {
    k = 0;
    if (match("+"))
      k = 1;
    if (match("-")) {
      menos = -menos;
      k = 1;
    }
  }
  if (isdigit(car_act) == 0)
    return 0;
  if (car_act == '0') {
    while (car_act == '0') obt_car();
    if (toupper(car_act) == 'X') {
      obt_car();
      while (isxdigit(car_act)) {
        c = toupper(obt_car()) - '0';
        if (c > 9) c = c - 7;
        k = (k << 4) | c;
      }
    } else {
      while ((car_act >= '0') && (car_act <= '7'))
        k = k * 8 + (obt_car() - '0');
    }
  } else {
    while (isdigit(car_act))
      k = k * 10 + (obt_car() - '0');
  }
  if (toupper(car_act) == 'L')
    obt_car();
  if (menos < 0)
    k = -k;
  val[0] = k;
  return 1;
}

int cad_caracteres(val)
  int val[];
{
  int k;

  k = 0;
  if (match("'") == 0)
    return 0;
  while (car_act != 39)
    k = k * 256 + (caracter_literal() & 255);
  ++pos_linea;
  val[0] = k;
  return 1;
}

int cad_literal(val)
  int val[];
{
  unsigned char c;

  if (match("\"") == 0)
    return 0;
  val[0] = ap_lit;
  while (car_act != '"') {
    if (car_act == 0)
      break;
    if (ap_lit >= MAX_LITS) {
      error("String space is full");
    while (match("\"") == 0)
      if (obt_car() == 0)
        break;
      return 1;
    }
    lits[ap_lit++] = caracter_literal();
  }
  obt_car();
  lits[ap_lit++] = 0;
  return 1;
}

int caracter_literal()
{
  int i, oct;

  if ((car_act != 92) || (prox_car() == 0))
    return obt_car();
  obt_car();
  if (car_act == 'n') {
    ++pos_linea;
    return 10;
  }
  if (car_act == 't') {
    ++pos_linea;
    return 9;
  }
  if (car_act == 'b') {
    ++pos_linea;
    return 8;
  }
  if (car_act == 'f') {
    ++pos_linea;
    return 12;
  }
  if (car_act == 'r') {
    ++pos_linea;
    return 13;
  }
  if (car_act == 'x') {
    ++pos_linea;
    while (isxdigit(car_act)) {
      i = toupper(obt_car()) - '0';
      if (i > 9) i = i - 7;
      oct = (oct << 4) | i;
    }
    return oct;
  }
  i = 3;
  oct = 0;
  while ((i-- > 0) && (car_act >= '0') && (car_act <= '7'))
    oct = (oct << 3) + obt_car() - '0';
  if (i == 2)
    return obt_car();
  else
    return oct;
}

/*
** Funciones de conversión y chequeo de tipos.
*/

void checa_entero(tipo)
  unsigned char *tipo;
{
  if (*tipo != CHAR && *tipo != SHORT && *tipo != INT
   && *tipo != USHORT && *tipo != UINT)
    error("It isn't an integer type");
}

void checa_numerico(tipo)
  unsigned char *tipo;
{
  if (*tipo != CHAR && *tipo != SHORT && *tipo != INT
   && *tipo != USHORT && *tipo != UINT && *tipo != DOUBLE
   && *tipo != FLOAT && *tipo != APUNTADOR)
    error("It isn't a numeric type");
}

void checa_entero_o_apuntador(tipo)
  unsigned char *tipo;
{
  if (*tipo != CHAR && *tipo != SHORT && *tipo != INT
   && *tipo != USHORT && *tipo != UINT && *tipo != APUNTADOR)
    error("It isn't an integer type");
}

void compara_no_cero(tipo)
  unsigned char *tipo;
{
  struct nodo *izq;

  if (*tipo != DOUBLE && *tipo != FLOAT)
    return;
  izq = ultimo_nodo;
  if (*tipo == DOUBLE)
    crea_nodo(N_CEROPF, NULL, NULL, 0);
  else
    crea_nodo(N_CEROF, NULL, NULL, 0);
  crea_nodo(N_IGUALPF, izq, ultimo_nodo, 0);
  crea_nodo(N_NOT, ultimo_nodo, NULL, 0);
}

void compara_cero(tipo)
  unsigned char *tipo;
{
  struct nodo *izq;

  izq = ultimo_nodo;
  if (*tipo == DOUBLE)
    crea_nodo(N_CEROPF, NULL, NULL, 0);
  else
    crea_nodo(N_CEROF, NULL, NULL, 0);
  crea_nodo(N_IGUALPF, izq, ultimo_nodo, 0);
}

void convierte_tipo(nodo, tipo_original, nuevo_tipo)
  struct nodo **nodo;
  unsigned char *tipo_original, *nuevo_tipo;
{
  if (*tipo_original == STRUCT && *nuevo_tipo != STRUCT)
    error("Cannot convert struct");
  else if (*tipo_original != STRUCT && *nuevo_tipo == STRUCT)
    error("Cannot convert to struct");
  else if (*tipo_original == VOID)
    error("Cannot convert from void");
  else if (*tipo_original == APUNTADOR &&
          (*nuevo_tipo == DOUBLE || *nuevo_tipo == FLOAT))
    error("Cannot convert pointer to float/double");
  else if (*nuevo_tipo == APUNTADOR &&
          (*tipo_original == DOUBLE || *tipo_original == FLOAT))
    error("Cannot convert float/double to pointer");
  else {
    if (*tipo_original == DOUBLE && *nuevo_tipo == DOUBLE)
      return;
    if (*tipo_original == FLOAT && *nuevo_tipo == FLOAT)
      return;
    if (*tipo_original == DOUBLE && *nuevo_tipo == FLOAT) {
      crea_nodo(N_CONVDF, *nodo, NULL, 0);
      *nodo = ultimo_nodo;
    } else if (*tipo_original == FLOAT && *nuevo_tipo == DOUBLE) {
      crea_nodo(N_CONVFD, *nodo, NULL, 0);
      *nodo = ultimo_nodo;
    } else if (*tipo_original == DOUBLE || *tipo_original == FLOAT) {
      crea_nodo(N_PFENT, *nodo, NULL, 0);
      *nodo = ultimo_nodo;
    } else if (*nuevo_tipo == FLOAT) {
      crea_nodo(N_ENTF, *nodo, NULL, 0);
      *nodo = ultimo_nodo;
    } else if (*nuevo_tipo == DOUBLE) {
      crea_nodo(N_ENTPF, *nodo, NULL, 0);
      *nodo = ultimo_nodo;
    }
  }
}

int haz_compatible(nodo_izq, info_izq, nodo_der, info_der)
  struct nodo **nodo_izq, **nodo_der;
  unsigned char **info_izq;
  unsigned char **info_der;
{
  unsigned char *tipo_izq, *tipo_der;

  tipo_izq = *info_izq;
  tipo_der = *info_der;
  if (*tipo_izq == STRUCT || *tipo_der == STRUCT)
    error("Operations with struct are disallowed");
  if (*tipo_izq == FLOAT && *tipo_der == FLOAT)
    return 1;
  if (*tipo_izq == DOUBLE && *tipo_der == DOUBLE)
    return 1;
  if ((*tipo_izq == FLOAT && *tipo_der == DOUBLE)
  || (*tipo_izq == DOUBLE && *tipo_der == FLOAT)) {
    if (*tipo_izq == FLOAT) {
      crea_nodo(N_CONVFD, *nodo_izq, NULL, 0);
      *nodo_izq = ultimo_nodo;
    } else {
      crea_nodo(N_CONVFD, *nodo_der, NULL, 0);
      *nodo_der = ultimo_nodo;
    }
    *info_izq = t_double;
    return 1;
  }
  if (*tipo_izq == FLOAT || *tipo_der == FLOAT) {
    if (*tipo_izq == FLOAT) {
      crea_nodo(N_ENTF, *nodo_der, NULL, 0);
      *nodo_der = ultimo_nodo;
    } else {
      crea_nodo(N_ENTF, *nodo_izq, NULL, 0);
      *nodo_izq = ultimo_nodo;
    }
    *info_izq = t_float;
    return 1;
  }
  if (*tipo_izq == DOUBLE || *tipo_der == DOUBLE) {
    if (*tipo_izq == DOUBLE) {
      crea_nodo(N_ENTPF, *nodo_der, NULL, 0);
      *nodo_der = ultimo_nodo;
    } else {
      crea_nodo(N_ENTPF, *nodo_izq, NULL, 0);
      *nodo_izq = ultimo_nodo;
    }
    *info_izq = t_double;
    return 1;
  }
  if ((*tipo_izq == UINT || *tipo_der == UINT)
  && (*tipo_izq != APUNTADOR && *tipo_der != APUNTADOR)
  && (*tipo_izq != MATRIZ && *tipo_der != MATRIZ))
    *info_izq = *info_der = t_uint;
  return 0;
}
