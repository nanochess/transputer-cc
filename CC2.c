/*
** Compilador de C para el G10
**
** por Oscar Toledo Gutiérrez.
**
** (c) Oscar Toledo G.1995.
**
** Creación: 26 de junio de 1995.
** Revisión: 27 de julio de 1995. Agrego comillas a los nombres, gracias a la
**                                nueva ampliación del compilador.
** Revisión: 23 de agosto de 1995. Incluyo el camino al directorio /c/.
** Revisión: 22 de noviembre de 1995. Incluyo el camino a la unidad c:
*/

#define NULL         0

int entrada,            /* Archivo de entrada */
    salida,             /* Archivo de salida */
    entrada2;           /* Archivo #include */
int desvio_salida;

/* ccinter.c */
void inicializa();
void color();
void cancela();
void reporta_errores();
void presentacion();
void opciones();
void abre_salida();
void abre_entrada();
void nuevo_archivo();
void p_include();
void fin_include();
void cierra_salida();

/* ccanasin.c */
void analiza();
void decl_typedef();
void decl_glb();
void decl_loc();
void inic_loc();
int p_tipo_1();
int p_tipo_2();
void copia_tipo();
void guarda_tipo();
int p_tipo_3();
void p_estructura();
void p_enumerador();
int tam_tipo();
int subindice();
void nueva_func();
void tipos_args();
void ordena_args();
int sentencia();
void punto_y_coma();
void p_bloque();
void s_if();
void s_while();
void s_do();
void s_for();
void s_switch();
void s_case();
void s_default();
void s_goto();
int p_etiqueta();
int agrega_etiqueta();
void s_return();
void s_break();
void s_cont();
int fin_sentencia();
void nombre_ilegal();
void redefinido();
void pide();

/* ccvarios.c */
void nuevo_bucle();
struct nombres *nueva_glb();
struct nombres *nueva_loc();
struct rotulo *nueva_estructura();
struct miembro *nuevo_miembro();
void nuevo_enum();
struct nombres *busca_glb();
struct nombres *busca_loc();
struct rotulo *busca_estructura();
struct miembro *busca_miembro();
struct enumerador *busca_enum();
int nombre_legal();
void mensaje();
int prox_car();
int obt_car();
void descarta();
int lee_car();
void lee_linea();
void preprocesa();
void primer_paso();
int almacena_car();
void pp_espacios();
void pp_comillas();
void pp_apostrofe();
void pp_comentarios();
void nueva_macro();
void borra_macro();
int pone_macro();
int busca_macro();
int emite_car();
void emite_nueva_linea();
void error();
void emite_linea();
void emite_texto();
int encuentra();
int streq();
int astreq();
int match();
int amatch();
void espacios();
int lee_entero();
void escribe_entero();
void emite_numero();
int letra();
int alfanum();
void basura();

/* ccexpr.c */
void evalua_arbol();
unsigned char *expresion();
int expr_constante();
unsigned char *almacena_expresion();
int nivel0();
int nivel1();
int nivel2();
int nivel3();
int nivel4();
int nivel5();
int nivel6();
int nivel7();
int nivel8();
int nivel9();
void nivel9eval();
void nivel9op();
int nivel10();
int nivel11();
int nivel12();
int nivel13();
int nivel13ap();
int nivel13dir();
void nivel13inc();
void nivel13dec();
void nivel13pinc();
void nivel13pdec();
int primaria();
void req_valorl();
void llama_funcion();
void carga_valor();
void dir_var_loc();
void dir_var_glb();
void dir_func();
void enlace();
int dobla();
void prueba();
int constante();
int numero_real();
int numero();
int cad_caracteres();
int cad_literal();
int caracter_literal();
void checa_entero();
void checa_numerico();
void checa_entero_o_apuntador();
void compara_no_cero();
void compara_cero();
void convierte_tipo();
int haz_compatible();

/* ccgencod.c */
void libera_arbol();
void crea_nodo();
void gen_codigo();
void etiqueta();
void gen_oper();
void gen_nodo();
void corto_circuito();
void accesa_nodo();
void estructura();
void carga();
void almacena();
void copia_resultado();
void asigna();
void ins();
void comentario();
void prologo();
void epilogo();
void libreria();
void emite_nombre();
void salva();
void recupera();
void copia_reg();
void llamada();
void retorno();
void salto();
void salta_si_falso();
void salto_no_int();
void emite_etiq();
void dos_puntos();
void def_byte();
int desp_pila();
void compara_y_salta();
void vacia_lits();

#define MAX_INCL     50

struct {
    int entrada;
    struct variable *funcion_actual;
    int comienzo_funcion;
    int linea_actual;
    int dentro_funcion;
} incl[MAX_INCL];

/*
** Desvia la salida a la consola.
*/
void hacia_consola()
{
  desvio_salida = salida;
  salida = 0;
  color(15);
}

/*
** Regresa la salida al archivo.
*/
void hacia_archivo()
{
  if (desvio_salida)
    salida = desvio_salida;
  desvio_salida = 0;
}

/*
** Prueba si el caracter dado es un nËmero.
*/
int isdigit(c)
  int c;
{
  return ((c >= '0') && (c <= '9'));
}

/*
** Checa si es un nËmero hexadecimal.
*/
int isxdigit(c)
  unsigned char c;
{
  return (((c >= '0') && (c <= '9')) ||
          ((c >= 'A') && (c <= 'F')) ||
          ((c >= 'a') && (c <= 'f')));
}

/*
** Checa si es un espacio.
*/
int isspace(c)
  unsigned char c;
{
  return (c == ' ') || (c == 9);
}

/*
** Conversión a mayúsculas.
*/
int toupper(c)
  unsigned char c;
{
  if ((c >= 'a') && (c <= 'z'))
    c = c + ('A' - 'a');
  return (c);
}

/*
** Retorna el tamaño de una cadena.
*/
int strlen(s)
  unsigned char *s;
{
  unsigned char *t;

  t = s;
  while (*s)
    s++;
  return (s - t);
}

/*
** Copia una cadena.
*/
void strcpy(destino, origen)
  unsigned char *destino, *origen;
{
  while (*destino++ = *origen++);
}

/*
** Concatena una cadena.
*/
void strcat(destino, origen)
  unsigned char *destino, *origen;
{
  while (*destino) ++destino;
  strcpy(destino, origen);
}

#include "CCvars.c"    /* Variables y definiciones.           */
#include "CCinter.c"   /* Interfaz con el usuario.            */
#include "CCanasin.c"  /* Análisis sintáctico de alto nivel.  */
#include "CCvarios.c"  /* Funciones de soporte.               */
#include "CCexpr.c"    /* Análisis sintáctico de expresiones. */
#include "CCgencod.c"  /* Generador de codigo.                */
