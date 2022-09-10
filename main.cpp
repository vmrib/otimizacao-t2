#include <algorithm>
#include <chrono>
#include <functional>
#include <iostream>
#include <set>
#include <tuple>
#include <vector>
#include <unistd.h>
#include <utility>

using namespace std;

// ----------------------------------------------------------------------------------
// Estrutura que define um ator
struct Ator
{
    uint id;          // "número" do ator (1..m)
    uint valor;       // valor cobrado
    set<uint> grupos; // grupos que ele faz parte
};

/**
 * A TAD set do C++ armazena seus elementos em ordem. É possível criar um set
 * com uma função de comparação customizada para seu algoritmo de sort interno.
 *
 * Nesse caso, os atores do conjunto estão sendo armazenados em ordem crescente
 * de valor cobrado. Útil para as funções de bound.
 */

// Função de comparação para ordenação de SetAtores. Odenação do menor valor para
// o maior
bool cmp_menor_valor(const Ator &a1, const Ator &a2);

// Define tipo customizado SetAtores para um conjunto de atores
using SetAtores = set<Ator, decltype(&cmp_menor_valor)>;

// Estrutura que define uma escolha. Uma escolha pode ser entendida como um nó da
// árvore de busca do algoritmo Branch and Bound.
struct Escolha
{
    SetAtores escolhidos; // conjunto de atores escolhidos
    SetAtores faltantes;  // conjunto de atores faltantes
    uint custo;           // custo total dos atores escolhidos
    uint limitante;       // bound da escolha. Utilizado no Branch and Bound

    // "Burocracia" para garantir que os conjuntos de atores sejam vinculados à
    // função de comparação cmp_menor_valor
    Escolha() : escolhidos(cmp_menor_valor), faltantes(cmp_menor_valor) {}
};

// Semelhante ao conjunto SetAtores. O conjunto de escolhas SetEscolhas é um set
// customizado cujo elementos são ordenados pelo limitante da escolha.
// Útil ao realizarmos corte or otimalidade.
bool cmp_menor_limitante(const Escolha &e1, const Escolha &e2);
using SetEscolhas = set<Escolha, decltype(&cmp_menor_limitante)>;

// ----------------------------------------------------------------------------------
/**
 * Função de bound dada pelo professor.
 * Recebe um conjunto de atores escolhidos e um conjunto de atores faltantes.
 * Retorna um limitante mínimo para o custo dos atores.
 */
uint bound_professor(SetAtores &escolhidos, SetAtores &faltantes);

/**
 * Função de bound criada por mim.
 * Recebe um conjunto de atores escolhidos e um conjunto de atores faltantes.
 * Retorna um limitante mínimo para o custo dos atores.
 */
uint bound_meu(SetAtores &escolhidos, SetAtores &faltantes);

/**
 * Função principal do Branch and Bound.
 * Recebe uma escolha, a qual irá realizar recursão (escolha <=> nó da árvore de busca)
 * e a função de bound.
 * Retorna a melhor escolha obtida na variável global melhor_escolha.
 */
void branch_and_bound(Escolha escolha,
                      function<uint(SetAtores &, SetAtores &)> bound);

/**
 * Verifica se a escolha atual contém uma solução viável.
 * Retorna true caso contenha, false caso contrário.
 */
bool escolha_viavel(Escolha &escolha);

/**
 * Gera um conjunto com as pŕoximas escolhas para "visitar", dada a escolha atual
 * Recebe uma escolha e uma função de bound.
 */
SetEscolhas computar_prox_escolhas(Escolha &escolha,
                                   function<uint(SetAtores &, SetAtores &)> bound);

// ----------------------------------------------------------------------------------
// Variáveis globais
uint l, m, n;                      // Total de grupos, atores e personagens
SetAtores atores(cmp_menor_valor); // Conjunto om todos os atores
Escolha melhor_escolha;            // Melhor escolha obtida

bool corte_otimalidade = true; // Flag para (des)habilitar corte por otimalidade
bool corte_viabilidade = true; // Flag para (des)habilitar corte por viabilidade

uint nos_arvore = 0;    // Total de nós gerados por computar_prox_escolhas()
uint nos_visitados = 0; // Total de nós efetivamente visitados.

// ----------------------------------------------------------------------------------
// Funcões para debug de código
#ifdef DEBUG

void debug(const char *msg);
void debug(const uint s, const char *nome = ":");
void debug(const set<uint> &s, const char *nome = ":");
void debug(const Ator &a, const char *nome = ":");
void debug(const Escolha &e, const char *nome = ":");
void debug(const SetAtores &s, const char *nome = ":");
void debug(const SetEscolhas &s, const char *nome = ":");

#else

template <typename T>
void debug(const T s, const char *_ = "")
{
}
#endif

// ----------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    int arg;
    opterr = 0;
    auto bound = &bound_meu;

    // getopt() fornecida pelo GNU para parsing de argumentos do programa
    // https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html
    while ((arg = getopt(argc, argv, "aof")) != -1)
    {
        switch (arg)
        {
        case 'a':
            bound = &bound_professor;
            break;

        case 'o':
            corte_otimalidade = false;
            break;

        case 'f':
            corte_viabilidade = false;
            break;

        default:
            break;
        }
    }

    // Leitura das entradas
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

    // Inicia "cronômetro" para medição de tempo gasto pelo algoritmo
    auto start = chrono::high_resolution_clock::now();

    // Na escolha inicial (nó raiz) o conjunto dos escolhidos é vazio e
    // o conjunto dos faltantes contém todos os atores
    Escolha escolha_inicial;
    escolha_inicial.faltantes = atores;
    escolha_inicial.custo = 0;

    // Configuração para início da recursão do branch_and_bound
    melhor_escolha = escolha_inicial;
    branch_and_bound(escolha_inicial, bound);

    // Para o "cronômetro" e armazena o tempo gasto pelo algoritmo
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);

    debug(melhor_escolha, "main:melhor_escolha");

    // Caso a melhor escolha seja igual à escolha inicial, então sabemos que
    // o branch_and_bound não conseguiu encontrar uma solução viável para o
    // problema. Na escolha inicial, o conjunto de atores escolhidos é vazio
    if (melhor_escolha.escolhidos.empty())
    {
        cout << "Inviavel" << endl;
        return 1;
    }

    // função lambda para comparação de ids dos atores. O vetor escolhidos contém
    // os atores escolhidos na ordem crescente de ids
    auto cmp_id = [](const Ator &a1, const Ator &a2)
    {
        return a1.id < a2.id;
    };

    // Idealmente poderia ter sido criado um conjunto ao invés de um vetor, mas
    // por algum motivo o compilador dava sempre problema, e não consegui arranjar
    // outra solução.
    vector<Ator> escolhidos(melhor_escolha.escolhidos.begin(), melhor_escolha.escolhidos.end());
    sort(escolhidos.begin(), escolhidos.end(), cmp_id);

    // id do último ator
    auto ultimo_id = escolhidos.rbegin()->id;

    // Imprime atores na ordem dos ids
    for (auto ator = escolhidos.begin(); ator->id != ultimo_id; ator++)
        cout << ator->id << " ";

    cout << ultimo_id << endl;
    cout << melhor_escolha.custo << endl;

    cerr << "Total de nos da arvore: " << nos_arvore << endl;
    cerr << "Total de nos visitados: " << nos_visitados << endl;
    cerr << "Tempo total gasto: " << duration.count();
    cerr << " us" << endl;

    return 0;
}

// ----------------------------------------------------------------------------------
/**
 * Função utilizada por SetAtores para ordenação dos atores pelo valor cobrado
 */
bool cmp_menor_valor(const Ator &a1, const Ator &a2)
{
    return a1.valor <= a2.valor;
}

/**
 * Função utilizada por SetEscolhas para ordenação das escolhas pelo limitante
 */
bool cmp_menor_limitante(const Escolha &e1, const Escolha &e2)
{
    return e1.limitante <= e2.limitante;
}

/**
 * Função de bound dada pelo professor.
 * Recebe um conjunto de atores escolhidos e um conjunto de atores faltantes.
 * Retorna um limitante mínimo para o custo dos atores.
 */
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

/**
 * Função de bound criada por mim.
 * Recebe um conjunto de atores escolhidos e um conjunto de atores faltantes.
 * Retorna um limitante mínimo para o custo dos atores.
 */
uint bound_meu(SetAtores &escolhidos, SetAtores &faltantes)
{
    // Detalhes do algoritmo no relatório
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

/**
 * Função principal do Branch and Bound.
 * Recebe uma escolha, a qual irá realizar recursão (escolha <=> nó da árvore de busca)
 * e a função de bound.
 * Retorna a melhor escolha obtida na variável global melhor_escolha.
 */
void branch_and_bound(Escolha escolha, function<uint(SetAtores &, SetAtores &)> bound)
{
    // se for uma escolha viável (ie. contém solução viável) existe possibilidade de
    // atualizar a melhor escolha obtida até então
    if (escolha_viavel(escolha))
    {
        // debug(escolha, "branch_and_bound:escolha (viavel)");

        // se a escolha atual for melhor que a obtida ou ainda não temos a melhor escolha
        if (escolha.custo < melhor_escolha.custo || melhor_escolha.escolhidos.empty())
            melhor_escolha = escolha;
        // debug(melhor_escolha, "branch_and_bound:melhor_escolha");
    }

    auto prox_escolhas = computar_prox_escolhas(escolha, bound);
    debug(prox_escolhas, "branch_and_bound:prox_escolhas");
    nos_arvore += prox_escolhas.size();

    for (auto &escolha : prox_escolhas)
    {
        debug(escolha, "branch_and_bound:escolha");

        // obs: caso a melhor_escolha ainda não tenha sido definida, então devemos obrigatoriamente realizar
        // a recursão.
        if (escolha.limitante >= melhor_escolha.custo && !melhor_escolha.escolhidos.empty() && corte_otimalidade)
        {
            debug("branch_and_bound: ramo cortado por otimalidade");
            return;
        }
        else if (escolha.escolhidos.size() > n && corte_viabilidade)
        {
            debug("branch_and_bound: ramo cortado por viabilidade");
            return;
        }
        else
        {
            debug("branch_and_bound: recursao na escolha");
            branch_and_bound(escolha, bound);
            nos_visitados++;
        }
    }
}

/**
 * Verifica se a escolha atual contém uma solução viável.
 * Retorna true caso contenha, false caso contrário.
 */
bool escolha_viavel(Escolha &escolha)
{
    // só é solução viável se o conjunto dos escolhidos for igual ao número de personagens
    if (escolha.escolhidos.size() != n)
        return false;

    // só é solução viável caso os atores escolhidos contemplem todos os grupos
    set<uint> grupos_escolhidos;

    for (auto &ator : escolha.escolhidos)
        for (auto &grupo : ator.grupos)
            grupos_escolhidos.insert(grupo);

    debug(grupos_escolhidos.size(), "escolha_viavel:grupos_escolhidos.size()");
    return grupos_escolhidos.size() == l;
}

/**
 * Gera um conjunto com as pŕoximas escolhas para "visitar", dada a escolha atual
 * Recebe uma escolha e uma função de bound.
 */
SetEscolhas computar_prox_escolhas(Escolha &escolha, function<uint(SetAtores &, SetAtores &)> bound)
{
    /**
     * Para computar o conjunto das próximas escolhas inserimos ao conjunto dos escolhidos
     * um ator do conjunto dos faltantes. Cada escolha é um ator diferente retirado dos
     * faltantes.
     * Exemplo:
     *
     * escolha atual
     * escolhidos: {1 2}, faltantes: {3 4 5}
     *
     * proximas escolhas:
     * {
     *      { escolhidos: {1 2 3}, faltantes: {4 5} }
     *      { escolhidos: {1 2 4}, faltantes: {3 5} }
     *      { escolhidos: {1 2 5}, faltantes: {3 4} }
     * }
     */
    SetEscolhas proximas(cmp_menor_limitante);

    for (auto &faltante : escolha.faltantes)
    {
        // função lambda para ser usada por find_if
        auto match_ator = [&faltante](const Ator &a)
        {
            return a.id == faltante.id;
        };

        // para a próxima escolha retiramos um ator do conjunto dos faltantes e
        // inserimos no conjunto dos escolhidos
        Escolha prox = escolha;

        prox.escolhidos.insert(faltante);

        /**
         * Para deletar um ator do conjunto de atores é necessário "casar" o id do ator.
         * Por isso devemos encontrar o ator cujo id queremos remover e então pasar o
         *  resultado para o método erase(). erase() recebe um iterador para o elemnto a
         *  ser deletado. Esse iterador é retornado por find_if().
         */
        prox.faltantes.erase(find_if(prox.faltantes.begin(), prox.faltantes.end(), match_ator));

        // cálculo do custo da próxima escolha
        prox.custo = 0;
        for (auto &ator : prox.escolhidos)
            prox.custo += ator.valor;

        // cálculo do limitante da próxima escolha
        prox.limitante = bound(prox.escolhidos, prox.faltantes);

        proximas.insert(move(prox));
    }

    debug(proximas, "computar_prox_escolhas:proximas");
    return proximas;
}

// Funções para debug
#ifdef DEBUG

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

#endif