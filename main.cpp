#include <algorithm>
#include <functional>
#include <iostream>
#include <set>
#include <tuple>
#include <vector>
#include <utility>

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

    Escolha() : escolhidos(cmp_menor_valor), faltantes(cmp_menor_valor) {}
};

bool cmp_menor_limitante(const Escolha &e1, const Escolha &e2);
using SetEscolhas = set<Escolha, decltype(&cmp_menor_limitante)>;

uint bound_professor(SetAtores &escolhidos, SetAtores &faltantes);
uint bound_meu(SetAtores &escolhidos, SetAtores &faltantes);

void branch_and_bound(Escolha escolha, function<uint(SetAtores &, SetAtores &)> bound);

bool escolha_viavel(Escolha &escolha);
SetEscolhas computar_prox_escolhas(Escolha &escolha, function<uint(SetAtores &, SetAtores &)> bound);

uint l, m, n;
SetAtores atores(cmp_menor_valor);
Escolha melhor_escolha;

#ifdef DEBUG

void debug(const char *msg);
void debug(const uint s, const char *nome = ":");
void debug(const set<uint> &s, const char *nome = ":");
void debug(const Ator &a, const char *nome = ":");
void debug(const Escolha &e, const char *nome = ":");
void debug(const SetAtores &s, const char *nome = ":");
void debug(const SetEscolhas &s, const char *nome = ":");

void debug(const char *msg)
{
    cout << msg << "\n";
}

void debug(const uint s, const char *nome)
{
    cout << nome << ": " << s << "\n";
}

void debug(const set<uint> &s, const char *nome)
{
    cout << nome << ": ( ";
    for (auto &&e : s)
        cout << e << " ";
    cout << ")\n";
}

void debug(const Ator &a, const char *nome)
{
    cout << nome;
    cout << ": { .id = " << a.id;
    cout << ", .valor = " << a.valor;
    cout << ", .grupos = ";
    debug(a.grupos);
    cout << " }\n";
}

void debug(const Escolha &e, const char *nome)
{
    cout << nome << ": {";
    cout << "\n\t.escolhidos = ";
    debug(e.escolhidos);
    cout << "\n\t.faltantes = ";
    debug(e.faltantes);
    cout << "\n\t.custo = " << e.custo;
    cout << "\n\t.limitante = " << e.limitante;
    cout << "\n}\n";
}

void debug(const SetAtores &s, const char *nome)
{
    cout << nome << ": (";
    for (auto &&e : s)
    {
        cout << "\n\t";
        debug(e);
    }
    cout << "\n)\n";
}

void debug(const SetEscolhas &s, const char *nome)
{
    cout << nome << ": (";
    for (auto &&e : s)
    {
        cout << "\n\t";
        debug(e);
    }
    cout << "\n)\n";
}

#else

template <typename T>
void debug(const T s, const char *_ = "")
{
}
#endif

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

        atores.insert(move(ator));
    }

    debug(atores, "main:atores");

    Escolha escolha_inicial;
    escolha_inicial.faltantes = atores;
    escolha_inicial.custo = 0;

    melhor_escolha = escolha_inicial;

    branch_and_bound(escolha_inicial, &bound_professor);

    debug(melhor_escolha, "main:melhor_escolha");

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
    return a1.valor <= a2.valor;
}

bool cmp_menor_limitante(const Escolha &e1, const Escolha &e2)
{
    return e1.limitante <= e2.limitante;
}

uint bound_professor(SetAtores &escolhidos, SetAtores &faltantes)
{
    uint soma_va = 0;
    for (auto &ator : escolhidos)
        soma_va += ator.valor;

    auto min_vaf = faltantes.begin()->valor;
    uint limitante = soma_va + (n - escolhidos.size()) * min_vaf;

    debug(escolhidos, "bound_professor:escolhidos");
    debug(faltantes, "bound_professor:faltantes");
    debug(limitante, "bound_professor:limitante");

    return limitante;
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

    uint limitante = soma_va + soma_vaf;

    debug(escolhidos, "bound_meu:escolhidos");
    debug(faltantes, "bound_meu:faltantes");
    debug(limitante, "bound_meu:limitante");

    return limitante;
}

void branch_and_bound(Escolha escolha, function<uint(SetAtores &, SetAtores &)> bound)
{
    // static Escolha melhor_escolha;

    if (escolha_viavel(escolha))
    {
        debug(escolha, "branch_and_bound:escolha (viavel)");
        if (escolha.custo < melhor_escolha.custo || melhor_escolha.escolhidos.empty())
            melhor_escolha = escolha;
        debug(melhor_escolha, "branch_and_bound:melhor_escolha");
    }

    auto prox_escolhas = computar_prox_escolhas(escolha, bound);
    debug(prox_escolhas, "branch_and_bound:prox_escolhas");

    for (auto &escolha : prox_escolhas)
    {
        debug(escolha, "branch_and_bound:escolha");
        if (escolha.limitante >= melhor_escolha.custo && !melhor_escolha.escolhidos.empty())
        {
            debug("branch_and_bound: ramo cortado");
            continue;
        }
        else
        {
            debug("branch_and_bound: recursao na escolha");
            branch_and_bound(escolha, bound);
        }
    }

    // return melhor_escolha;
}

bool escolha_viavel(Escolha &escolha)
{
    if (escolha.escolhidos.size() != n)
        return false;

    set<uint> grupos_escolhidos;

    for (auto &ator : escolha.escolhidos)
        for (auto &grupo : ator.grupos)
            grupos_escolhidos.insert(grupo);

    debug(grupos_escolhidos.size(), "escolha_viavel:grupos_escolhidos.size()");
    return grupos_escolhidos.size() == l;
}

SetEscolhas computar_prox_escolhas(Escolha &escolha, function<uint(SetAtores &, SetAtores &)> bound)
{

    SetEscolhas proximas(cmp_menor_limitante);

    for (auto &faltante : escolha.faltantes)
    {
        auto match_ator = [&faltante](const Ator &a)
        {
            return a.id == faltante.id;
        };

        Escolha prox = escolha;

        prox.escolhidos.insert(faltante);
        prox.faltantes.erase(find_if(prox.faltantes.begin(), prox.faltantes.end(), match_ator));

        prox.custo = 0;
        for (auto &ator : prox.escolhidos)
            prox.custo += ator.valor;

        prox.limitante = bound(prox.escolhidos, prox.faltantes);

        proximas.insert(move(prox));
    }

    debug(proximas, "computar_prox_escolhas:proximas");
    return proximas;
}