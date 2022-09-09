// #include <algorithm>
// #include <bitset>
// #include <iostream>
// #include <set>

// using namespace std;

// struct Ator
// {
//     uint valor;
//     set<uint> grupos;
// };

// uint bound_professor(set<Ator> escolhidos, set<Ator> faltantes, uint n)
// {
//     uint soma_va = 0;

//     for (auto &&ator : escolhidos)
//         soma_va += ator.valor;

//     auto menor_valor = [](const Ator &a1, const Ator &a2)
//     {
//         return a1.valor < a2.valor;
//     };

//     uint min_vaf = min_element(faltantes.begin(), faltantes.end(), menor_valor)->valor;

//     return soma_va + (n - escolhidos.size()) * min_vaf;
// }

// uint bound_meu(set<Ator> escolhidos, set<Ator> faltantes, uint n)
// {
//     uint soma_va = 0;

//     for (auto &&ator : escolhidos)
//         soma_va += ator.valor;

//     uint p_faltantes = n - escolhidos.size();
//     uint soma_faltantes = 0;
//     for (auto ator = faltantes.begin(); ator != faltantes.end() && p_faltantes > 0; ator++, p_faltantes--)
//     {
//         soma_faltantes += ator->valor;
//     }

//     return soma_va +
// }

// int main(int argc, const char *argv[])
// {
//     set<Ator> atores;
//     set<uint> grupos, personagens;
//     uint l, m, n;

//     cin >> l >> m >> n;

//     for (uint i = 0; i < m; i++)
//     {
//         uint s;
//         Ator ator;
//         cin >> ator.valor >> s;

//         for (uint j = 0; j < s; j++)
//         {
//             uint grupo;
//             cin >> grupo;
//             ator.grupos.insert(grupo);
//         }
//     }

//     return 0;
// }
