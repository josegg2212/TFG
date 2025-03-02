#include <iostream>
#include "peglib.h"

using namespace std;
using namespace peg;

const char *grammar = R"(
    start <- command

    command <- COMMAND_KEYWORD command_content

    command_content <- tool / piece / object / measurement

    tool <- TOOL_ID
    piece <- PIECE_ID
    measurement <- MEASUREMENT_ID WS measurement_value?
    object <- OBJECTS

    measurement_value <- single_value / range_value / satisfied
    single_value <- VALUE WS unit?
    range_value <- "mínimo" min_value "máximo" max_value
    satisfied <- SATISFIED_KEYWORDS

    unit <- UNIT
    min_value <- VALUE WS unit?
    max_value <- VALUE WS unit?

    COMMAND_KEYWORD <- "cota" / "pieza" / "herramienta" / "abrir" / "cerrar" / "repetir" / "reiniciar" / "siguiente" / "quitar"
    TOOL_ID <- "sc" [0-9][0-9][0-9][0-9]
    MEASUREMENT_ID <- [A-Za-z] ([A-Za-z] / [0-9])?
    PIECE_ID <- [0-9]+
    OBJECTS <- "orden" / "sesión" / "plano" / "herramienta" / "cota" / "rango"

    VALUE <- [0-9]+ ([.,][0-9]+)?
    UNIT <- "milímetros" / "mm" / "pulgadas" / "in"
    SATISFIED_KEYWORDS <- "conforme" / "no conforme"

    WS <- [ \t]*
    %whitespace <- [ \t]*
    )";

int main()
{
    parser p(grammar);

    string input;

    while (1)
    {
        cout << "Introduce una frase: ";
        getline(cin, input);

        if (p.parse(input))
        {
            cout << "Comando válido: " << input << endl;
        }
        else
        {
            cout << "Entrada no reconocida." << endl;
        }
    }
    return 0;
}
