#include <algorithm>
#include <functional>
#include <iostream>
#include <set>
#include <tuple>
#include <vector>

using namespace std;

struct Ator
{
    uint id;
    uint valor;
    set<uint> grupos;
};

bool cmp_menor_valor(const Ator &a1, const Ator &a2);
using SetAtores = set<Ator, decltype(&cmp_menor_valor)>;

struct Escolha
{
    SetAtores escolhidos;
    SetAtores faltantes;
    uint custo;
    uint limitante;
};

bool cmp_menor_limitante(const Escolha &e1, const Escolha &e2);
using SetEscolhas = set<Escolha, decltype(&cmp_menor_limitante)>;

uint bound_professor(SetAtores &escolhidos, SetAtores &faltantes);
uint bound_meu(SetAtores &escolhidos, SetAtores &faltantes);

Escolha branch_and_bound(Escolha escolha, function<uint(SetAtores &, SetAtores &)> bound);

bool escolha_viavel(Escolha &escolha);
SetEscolhas computar_prox_escolhas(Escolha &escolha, function<uint(SetAtores &, SetAtores &)> bound);

uint l, m, n;
SetAtores atores;

int main(int argc, const char *argv[])
{
    cin >> l >> m >> n;

    for (uint i = 1; i <= m; i++)
    {
        Ator ator;
        uint s;

        ator.id = i;

        cin >> ator.valor >> s;

        for (uint j = 0; j < s; j++)
        {
            uint g;
            cin >> g;
            ator.grupos.insert(g);
        }

        atores.insert(ator);
    }

    Escolha escolha_inicial = {
        .faltantes = atores,
        .custo = 0,
    };

    auto melhor_escolha = branch_and_bound(escolha_inicial, &bound_meu);

    if (melhor_escolha.escolhidos.empty())
    {
        cout << "Inviavel" << endl;
        return 1;
    }

    auto cmp_id = [](const Ator &a1, const Ator &a2)
    {
        return a1.id < a2.id;
    };
    // set<Ator, decltype(&cmp_id)> escolhidos;

    vector<Ator> escolhidos(melhor_escolha.escolhidos.begin(), melhor_escolha.escolhidos.end());
    sort(escolhidos.begin(), escolhidos.end(), cmp_id);

    // for (auto &&ator : melhor_escolha.escolhidos)
    // {
    //     escolhidos.insert(ator);
    // }

    auto ultimo_id = escolhidos.rbegin()->id;

    for (auto ator = escolhidos.begin(); ator->id != ultimo_id; ator++)
        cout << ator->id << " ";

    cout << ultimo_id << endl;
    cout << melhor_escolha.custo << endl;

    return 0;
}

bool cmp_menor_valor(const Ator &a1, const Ator &a2)
{
    return a1.valor < a2.valor;
}

bool cmp_menor_limitante(const Escolha &e1, const Escolha &e2)
{
    return e1.limitante < e2.limitante;
}

uint bound_professor(SetAtores &escolhidos, SetAtores &faltantes)
{
    uint soma_va = 0;
    for (auto &ator : escolhidos)
        soma_va += ator.valor;

    auto min_vaf = faltantes.begin()->valor;

    return soma_va + (n - escolhidos.size()) * min_vaf;
}

uint bound_meu(SetAtores &escolhidos, SetAtores &faltantes)
{
    uint soma_va = 0;
    for (auto &ator : escolhidos)
        soma_va += ator.valor;

    uint soma_vaf = 0;
    uint p_faltantes = n - escolhidos.size();
    uint i = 0;
    for (auto ator = faltantes.begin(); ator != faltantes.end() && i < p_faltantes; ator++, i++)
        soma_vaf += ator->valor;

    return soma_va + soma_vaf;
}

Escolha branch_and_bound(Escolha escolha, function<uint(SetAtores &, SetAtores &)> bound)
{
    static Escolha melhor_escolha;

    if (escolha_viavel(escolha))
    {
        if (escolha.custo < melhor_escolha.custo || melhor_escolha.escolhidos.empty())
            melhor_escolha = escolha;
    }

    auto prox_escolhas = computar_prox_escolhas(escolha, bound);

    for (auto &escolha : prox_escolhas)
    {
        if (escolha.custo >= melhor_escolha.custo)
            return melhor_escolha;
        else
            return branch_and_bound(escolha, bound);
    }

    return melhor_escolha;
}

bool escolha_viavel(Escolha &escolha)
{
    set<uint> grupos_escolhidos;

    for (auto &ator : escolha.escolhidos)
        for (auto &grupo : ator.grupos)
            grupos_escolhidos.insert(grupo);

    return grupos_escolhidos.size() == l;
}

SetEscolhas computar_prox_escolhas(Escolha &escolha, function<uint(SetAtores &, SetAtores &)> bound)
{
    SetEscolhas proximas;

    for (auto &faltante : escolha.faltantes)
    {
        Escolha prox = escolha;

        prox.escolhidos.insert(faltante);
        prox.faltantes.erase(faltante);

        prox.custo = 0;
        for (auto &ator : prox.escolhidos)
            prox.custo += ator.valor;

        prox.limitante = bound(prox.escolhidos, prox.faltantes);

        proximas.insert(prox);
    }

    return proximas;
}