# Ark Programming Language

**Ark** es un lenguaje de programacion compilado, de tipado hibrido (inferido estatico + explicito), diseñado desde 0 en C++. Su objetivo es ser la mera vrg, con una sintaxis limpia, moderna e intuitiva con la potencia de C++ y asm puro (**x86-64 NASM**).

---

## Vision y Filosofia
* **Sintaxis Limpia:** Bloques definidos por identacion o estructura logica, sin abuso de llaves `{}` ni punto y coma obligatorios en cada respiro.
* **Performance Real:** Compila directamente a codigo maquina nativo (ELF64), sin maquinas virtuales ni interpretes intermedios.
* **Low Level Power:** Acceso directo a registros y memoria (eventualmente), pero con abstracciones comodas para el dia a dia.

## Pipeline de compilacion
Si estas leyendo esto, ya sabes como funciona un compilador. Ark sigue el flujo clasico:

1. **Source Code (.ark):** Entrada de texto plano.
2. **Lexer (Tokenizacion):** Convierte el texto en una corriente de de `Tokens` (Palabras clave, IDs, Literales).
3. **Parcer (AST):** Analiza la gramatica y construye un Arbol de Sintaxis Abstracta (AST) jerarquico.
4. **Code Generator (Backend):** Recorre el AST y emite instrucciones ensablador **NASM (x86-64)**.
5. **Assembler & Linker:** Usa `nasm` y `ld` para generar el binario ejecutable final.

---

## Instalacion

### Requisitos
- **Linux** (x84-64)
- **g++** (C++17 o superior)
- **NASM** (ensamblador)
- **ld** (linker)
- **git** (control de versiones)
```bash
# Ubuntu/debian
sudo apt install git g++ nasm build-essential

# Arch Linux
sudo pacman -S git gcc nasm
```

### Compilar compilador Ark
```bash
git clone https://github.com/Dev-DooM8002/Ark.git
cd Ark
chmod +x setup
./setup
sudo mv build/ark /usr/bin
cd ..
rm -rf Ark
```

---

## Uso
```bash
ark file.ark
./file
```

El compilador genera:
1. `output.asm` - codigo NASM
2. `output.o` - objeto ensamblado
2. `output` - ejecutable final

---

## Sintaxis

### Estructura basica
```ark
.data:
    // Tus variables globales o
    // inicializadas, desde el prinicipio
    // AQUI 
.sbox:
    // Tus funciones y clases,
    // AQUI

.start: // entry point
    // Tu codigo aqui
.end(codigo de retorno aqui)
```


### Comentarios
**Comentario In-Line:**
```ark
// Este es un comentario in-line 
```
**Comentario Multi-Line:**
```ark
<comment>
    Este
    Es un
    Comentario
    Multilinea.
<end>
```

### Variables

**Inferencia de tipos:**
```ark
var x: 42;          // int
var name: "Ark";    // str
var flag: true;     // bool
```

**Tipos explicitos:**
```ark
var int age: 1000;
var str lang: "Ark";
var bool flag: false;
```

### Input / Output (I/O)
```ark
pnl("hola"); // Print New Line
pil("hola"); // Print In Line

cin(varCont, "aqui pones tu prompt: "); // Console Input
cin(varCont);
// parametros de cin: var y prompr
// var es quien almacenara la entrada
```

### Operadores

**Operadores Aritmeticos:**
```ark
// +,-,*,/
```
**Operadores de Comparacion:**
```ark
// ==,!=,<,>,<=,>=
```
**Operadores Logicos:**
```ark
// and,&,or,|,not,!
```
**Operador de concatenacion:**
```ark
// <<
```
**Operador de asignacion:**
```ark
// :
```

### Estructuras de control

**Expresion :**
```ark
if (condicional):
    // Aqui va el cuerpo del if
.end
```
**Expresion if/else:**
```ark
if (condicional):
    // Aqui va el cuerpo del if
else:
    // aqui va el cuerpo del else
.end
```
**Expresion if/elif/else:**
```ark
if (condicional):
    // Aqui va el cuerpo del if
elif (condicional):
    // Aqui va el cuerpo del elif
elif (condicional):
    // Aqui va el cuerpo del elif
else:
    // aqui va el cuerpo del else
.end
```

### Bucles

**Bucle condicional (While Loop):**
```ark
// Itera mientras la condicion sea verdadera
loop (condicion):
    // Aqui va el cuerpo del loop
.end
```

**Bucle condicional (Until Loop):**
```ark
// Itera hasta q la condicion sea verdadera
uloop (condicion):
    // Aqui va el cuerpo del uloop
.end
```

**Bucle Infinito (Infinite Loop):**
```ark
// itera indefinidamente
loop:
    // Cuerpo del loop
.end
```
#### Controles de flujo
* **break** Break rompe bucle.
* **jump** Jump salta la iteracion actual dentro de un bucle.


### Funciones

**Funciones Siples:**
```ark
// funcion para operaciones simples
fn name(parametros): operacion;
```

**Funciones sin Retorno:**
```ark
fn name (Parametros):
    // Aqui va el cuerpo de la funcion
.end
```

**Funciones con retono:**
```ark
fn name (parametros) -> ret-type:
    // Aqui va el cuerpo de la funcion
.end(ret)
```