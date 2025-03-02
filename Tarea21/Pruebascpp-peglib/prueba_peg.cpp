#include <iostream>
#include "peglib.h"

using namespace std;
using namespace peg;

int main()
{
    const char *grammar = R"(
        comando <- accion WS objeto
        accion <- "enciende" / "apaga" / "sube" / "baja"
        objeto <- "la luz" / "el televisor" / "el volumen"
        WS <- [ \t]*
    )";

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
