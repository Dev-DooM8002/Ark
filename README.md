Name:
    Ark
Ext:
    .ark

Pipeline:
    1. Lexer (Tokenizador):
        Agarra el codigo fuente (Texto plano)
        y lo parte en pedasos con sisgnificado (Tokens)
            * Ejemplo:
                Input: var x = 10
                output: [keyword_var, id(x), op(=), int(10), semicolon(;)]
    
    2. Pareser (Analizador sinstactico):
        *Gramatica: conjunto de reglas q establesen como se combinan los simbolos.
        Agarra esos tokens y revisa q tengan sentido gramatical 
        y contruye un AST (Abstract syntax tree)
            * Ejemplo:
                input: [keyword_var, id(x), op(=), int(10), semicolon(;)]
                output: VarDec
                        |--Id("x")
                        |--Int(10)
    
    3. Analisis semantico:
        Revisa q los tipos coincidan (la menos de q el lenguaje lo permita)
        y que las variables existan
            *Ejemplo:
                text + 10: ERROR
                varUndifined + x: ERROR

                10 + 10: CORRECT
                x + x: CORRECT

    * Opcional:
        1. Generador de codigo Intermedio (IR)
        2. Optimizaciones (sobre IR)

    4. Generador de codigo (El backend):
        Generar codigo ASM (NASM) directamente, escribirlo en un .asm
        y luego ensamblarlo y linkearlo
            Output:
                Ejecutable

    5. Ejecutable:
        no ahi mucho q decir de este.
        solo ejecutalo y ve la magiiiaaaaaaaa :)



Ontologia:
    1. Tipado ??:
        -Custom:
            De q va este tipado custom ??
            Este tipado sera 'hyper-dinamico'
            contara con:
                Inferencia de tipos. (deteccion automatica, x parte del compilador)
                Tipo estatico explicito. (declarar con tipo explicitamente)
            Ejemplos:
                x = 100: Inferencia de tipos
                int x = 1: tipo estatico explicito

    2. Paradigma ??:
        Todos :)

    3. Compilado o interpretado ??:
        Hibrido :)

# +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ #
# +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ #
# +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ #

++ Ark Fase 1 ++

Inferencia: Estatica
Paradigma: Procedural
Implementacion: Compilada

Current Pipeline:
    1. Lexer
    2. Parser
    3. Semantic Analysis
    4. codegen (nasm)



// Comentario en Ark
// 1. ¿Cómo se declara una función main (punto de entrada)?
// 2. ¿Cómo declaramos variables inferidas y explicitas?
// 3. ¿Usamos llaves {}, identación (tipo Python) o palabras (begin/end)?

Comentarion in-line:
    // Este es un comentario in-line

Comentarios multi-line:
    <begin>
        Este
        es un
        comentario
        multi-linea
    <end>

Estructura: mixto:
    se usara delimitadores () para expresiones y [] para listas
    se utilizara la palabra .end o end() si ahi ret-value o end code
    e identacion estetica

Types (iniciales):
    int
    str
    char
    bool

Scopes:
    global: valida en todas partes
    local: valida solo dentro de su bloque

Varibles:
    Declaracion de variables explicitas:
        opciones de secuensias: keyword_var scope type name op_assign value semicolon
        opciones de secuensias: keyword_var type name op_assign value semicolon
        opciones de secuensias: scope type name op_assign value semicolon
        opciones de secuensias: type name op_assign value semicolon
            *Ejemplos:
                var local int myVar: 32;
                var int myVar: 32;
                local int myVar: 32;
                int myVar: 32;

    Declaracion de variables inferidas:
        opciones de secuensias: keyword_var scope name op_assign value semicolon
        opciones de secuensias: keyword_var name op_assign value semicolon
        opciones de secuensias: scope name op_assign value semicolon
        opciones de secuensias: name op_assign value semicolon
            *Ejemplos:
                var local myVar: 32;
                var myVar: 32;
                local myVar: 32;
                myVar: 32;

Funciones:
    Funciones sin retorno:
        secuensia: keyword_fun name ( tipe(opcional): name ) : 
            Code Block
        .end

    Funciones con retorno:
        secuensia: keyword_fun name ( tipe(opcional): name ) -> ret-type : 
            Code Block
        .end(ret-value)

Estructuras de control (condicionales):
    Condicional if:
        secuencia: keyword_if ( Condicion ):
            Code Block
        .end

    Condicional if/else:
        secuencia: keyword_if ( Condicion ):
            Code Block
        secuencia: keyword_else :
            Code Block
        .end

    Condicional if/elif/else:
        secuencia: keyword_if ( Condicion ):
            Code Block
        secuencia: keyword_elif ( condition ):
            Code Block
        secuencia: keyword_else :
            Code Block
        .end


sintaxis principal del los archivos .ark:
.data: ( lugar "Opcional, para declaracion de variables globales)
    ejemplo:
    var global string myStr: "Hola";
    o
    var global string myStr > "Hola";
.sbox: ((sandbox guiño, guiño jaja) aqui ira todo lo externo al punto de entrada, como: funciones, clases y etc, etc)
    fun myFun (int: y) -> int:
        z: (y + 10)
    .end(z)
.start: (Declaracion de punto de entrada)
    y: 8;
    b: myFun(y)
    pnl(b) // pnl: print new line print \n jajajaj
.end(end code) (cierre de punto de entrada)



# +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ #
# +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ #
# +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ #


Pasa a produccion en Fase 1:
    (:) como operador de asignacion
    como stintaxis de variable(explicito):
        (keyword_var, type, id, assign, value, semicolon) 
    stintaxis de variable(inferido):
        (keyword_var, id, assign, value, semicolon) 
    (<begin>/<end>) como comentarios multi-linea