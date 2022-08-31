#include <bitset>
#include <iostream>
#include <set>

using namespace std;

struct Ator
{
    uint valor;
    set<uint> grupos;
};

int main(int argc, const char *argv[])
{
    set<Ator> atores;
    set<uint> grupos, personagens;
    uint l, m, n;

    cin >> l >> m >> n;

    for (uint i = 0; i < m; i++)
    {
        uint s;
        Ator ator;
        cin >> ator.valor >> s;

        for (uint j = 0; j < s; j++)
        {
            uint grupo;
            cin >> grupo;
            ator.grupos.insert(grupo);
        }
    }

    return 0;
}
