//
//  Graphe.cpp
//  Classe pour graphes orientés pondérés (non négativement) avec listes d'adjacence
//
//  Mario Marchand automne 2016.
//

#include "graphe.h"

using namespace std;

//! \brief Constructeur avec paramètre du nombre de sommets désiré
//! \param[in] p_nbSommets indique le nombre de sommets désiré
//! \post crée le vecteur de p_nbSommets de listes d'adjacence vides avec nbArcs=0
Graphe::Graphe(size_t p_nbSommets)
        : m_listesAdj(p_nbSommets), m_nbArcs(0)
{
}

//! \brief change le nombre de sommets du graphe
//! \param[in] p_nouvelleTaille indique le nouveau nombre de sommet
//! \post le graphe est un vecteur de p_nouvelleTaille de listes d'adjacence
//! \post les anciennes listes d'adjacence sont toujours présentes lorsque p_nouvelleTaille >= à l'ancienne taille
//! \post les dernières listes d'adjacence sont enlevées lorsque p_nouvelleTaille < à l'ancienne taille
//! \post nbArcs est diminué par le nombre d'arcs sortant des sommets à enlever si certaines listes d'adgacence sont supprimées
void Graphe::resize(size_t p_nouvelleTaille)
{
    if (p_nouvelleTaille < m_listesAdj.size()) //certaines listes d'adj seront supprimées
    {
        //diminuer nbArcs par le nb d'arcs sortant des sommets à enlever
        for(size_t i = p_nouvelleTaille; i < m_listesAdj.size(); ++i)
        {
            m_nbArcs -= m_listesAdj[i].size();
        }
    }
    m_listesAdj.resize(p_nouvelleTaille);
}

size_t Graphe::getNbSommets() const
{
    return m_listesAdj.size();
}

size_t Graphe::getNbArcs() const
{
    return m_nbArcs;
}

//! \brief ajoute un arc d'un poids donné dans le graphe
//! \param[in] i: le sommet origine de l'arc
//! \param[in] j: le sommet destination de l'arc
//! \param[in] poids: le poids de l'arc
//! \pre les sommets i et j doivent exister
//! \throws logic_error lorsque le sommet i ou le sommet j n'existe pas
//! \throws logic_error lorsque le poids == numeric_limits<unsigned int>::max()
void Graphe::ajouterArc(size_t i, size_t j, unsigned int poids)
{
    if (i >= m_listesAdj.size())
        throw logic_error("Graphe::ajouterArc(): tentative d'ajouter l'arc(i,j) avec un sommet i inexistant");
    if (j >= m_listesAdj.size())
        throw logic_error("Graphe::ajouterArc(): tentative d'ajouter l'arc(i,j) avec un sommet j inexistant");
    if (poids == numeric_limits<unsigned int>::max())
        throw logic_error("Graphe::ajouterArc(): valeur de poids interdite");
    m_listesAdj[i].emplace_back(Arc(j, poids));
    ++m_nbArcs;
}

//! \brief enlève un arc dans le graphe
//! \param[in] i: le sommet origine de l'arc
//! \param[in] j: le sommet destination de l'arc
//! \pre l'arc (i,j) et les sommets i et j dovent exister
//! \post enlève l'arc mais n'enlève jamais le sommet i
//! \throws logic_error lorsque le sommet i ou le sommet j n'existe pas
//! \throws logic_error lorsque l'arc n'existe pas
void Graphe::enleverArc(size_t i, size_t j)
{
    if (i >= m_listesAdj.size())
        throw logic_error("Graphe::enleverArc(): tentative d'enlever l'arc(i,j) avec un sommet i inexistant");
    if (j >= m_listesAdj.size())
        throw logic_error("Graphe::enleverArc(): tentative d'enlever l'arc(i,j) avec un sommet j inexistant");
    auto &liste = m_listesAdj[i];
    bool arc_enleve = false;
    if(liste.empty()) throw logic_error("Graphe:enleverArc(): m_listesAdj[i] est vide");
    for (auto itr = liste.end(); itr != liste.begin();) //on débute par la fin par choix
    {
        if ((--itr)->destination == j)
        {
            liste.erase(itr);
            arc_enleve = true;
            break;
        }
    }
    if (!arc_enleve)
        throw logic_error("Graphe::enleverArc: cet arc n'existe pas; donc impossible de l'enlever");
    --m_nbArcs;
}


unsigned int Graphe::getPoids(size_t i, size_t j) const
{
    if (i >= m_listesAdj.size()) throw logic_error("Graphe::getPoids(): l'incice i n,est pas un sommet existant");
    for (auto & arc : m_listesAdj[i])
    {
        if (arc.destination == j) return arc.poids;
    }
    throw logic_error("Graphe::getPoids(): l'arc(i,j) est inexistant");
}


//! \brief Version amméliorée de l'algorithme de Dijkstra permettant de trouver le plus court chemin entre p_origine et p_destination
//! \pre p_origine et p_destination doivent être des sommets du graphe
//! \return la longueur du plus court chemin est retournée
//! \param[out] le chemin est retourné (un seul noeud si p_destination == p_origine ou si p_destination est inatteignable)
//! \return la longueur du chemin (= numeric_limits<unsigned int>::max() si p_destination n'est pas atteignable)
//! \throws logic_error lorsque p_origine ou p_destination n'existe pas
unsigned int Graphe::plusCourtChemin(size_t p_origine, size_t p_destination, std::vector<size_t> &p_chemin) const
{
    typedef pair<int, int> iPair;


    p_chemin.clear();
    // algo de base
    if (p_origine >= m_listesAdj.size() || p_destination >= m_listesAdj.size())
        throw logic_error("Graphe::dijkstraP(): p_origine ou p_destination n'existe pas");

    if (p_origine == p_destination)
    {
        p_chemin.push_back(p_destination);
        return 0;
    }

    priority_queue< iPair, vector <iPair> , greater<iPair> > pq;
    vector<size_t > dist(m_listesAdj.size(), numeric_limits<size_t>::max());
    vector<size_t> predecesseur(m_listesAdj.size(), numeric_limits<size_t>::max());
    vector<bool> visite(m_listesAdj.size(), false);



    // Insert source itself in priority queue and initialize
    // its distance as 0.
    pq.push(make_pair(0, p_origine));
    dist[p_origine] = 0;

    /* Looping till priority queue becomes empty (or all
      distances are not finalized) */
    while (!pq.empty())
    {
        // The first vertex in pair is the minimum distance
        // vertex, extract it from priority queue.
        // vertex label is stored in second of pair (it
        // has to be done this way to keep the vertices
        // sorted distance (distance must be first item
        // in pair)
        int u = pq.top().second;
        pq.pop();

        // 'i' is used to get all adjacent vertices of a vertex
        if (u==p_destination) break;
        visite[u] = true;
        list<Arc>::const_iterator i;
        for (i =m_listesAdj[u].begin(); i !=m_listesAdj[u].end(); ++i)
        {
            // Get vertex label and weight of current adjacent
            // of u.
            int v = (*i).destination;
            int weight = (*i).poids;

            //  If there is shorted path to v through u.
            if (!visite[v] && dist[v] > dist[u] + weight)
            {
                // Updating distance of v
                dist[v] = dist[u] + weight;
                pq.push(make_pair(dist[v], v));
                predecesseur[v] = u;
            }
        }
    }
    stack<size_t> pileDuChemin;
    size_t numero = p_destination;
    pileDuChemin.push(numero);

    if (predecesseur[p_destination] == numeric_limits<size_t>::max()){
        p_chemin.push_back(p_destination);
        return numeric_limits<size_t>::max();
    }
    while (predecesseur[numero] != numeric_limits<size_t>::max())
    {
        numero = predecesseur[numero];
        pileDuChemin.push(numero);
    }
    while (!pileDuChemin.empty())
    {
        size_t temp = pileDuChemin.top();
        p_chemin.push_back(temp);
        pileDuChemin.pop();
    }
    cout << dist[p_destination] << endl;
    return dist[p_destination];

}


//    p_chemin.clear();
//
//    vector<unsigned int> distance(m_listesAdj.size(), numeric_limits<unsigned int>::max());
//    vector<size_t> predecesseur(m_listesAdj.size(), numeric_limits<size_t>::max());
//    vector<bool> solutionne(m_listesAdj.size(), false);
//
//    distance[p_origine] = 0;
//
//    set<size_t> q; //ensemble des noeuds non solutionnés en bordure des noeuds solutionnés;
//    q.insert(p_origine);
//
//    //Boucle principale: touver distance[] et predecesseur[]
//    while (!q.empty())
//    {
//        //trouver le noeud dans q tel que distance[noeud] est minimal
//        unsigned int min = numeric_limits<unsigned int>::max();
//        auto noeud_solution = numeric_limits<size_t>::max();
//        for(const auto & noeud : q)
//        {
//            if(distance[noeud] < min)
//            {
//                min = distance[noeud];
//                noeud_solution = noeud;
//            }
//        }
//        if (min == numeric_limits<unsigned int>::max()) break; //quitter la boucle : il est impossible de se rendre à destination
//
//        q.erase(noeud_solution); //enlever le noeud solutionné de q
//        solutionne[noeud_solution] = true; //indique qu'il est solutionné
//
//        if (noeud_solution == p_destination) break; //car on a obtenu distance[p_destination] et predecesseur[p_destination]
//
//        //relâcher les arcs sortant de noeud_solution (le noeud solutionné)
//        for (const auto & arc : m_listesAdj[noeud_solution])
//        {
//            if (q.find(arc.destination) == q.end() && !solutionne[arc.destination])
//                q.insert(arc.destination); //insertion dans les noeuds à traiter
//
//            unsigned int temp = distance[noeud_solution] + arc.poids;
//            if (temp < distance[arc.destination])
//            {
//                distance[arc.destination] = temp;
//                predecesseur[arc.destination] = noeud_solution;
//            }
//        }
//
//    }
//
//    //cas où l'on n'a pas de solution
//    if (predecesseur[p_destination] == numeric_limits<unsigned int>::max())
//    {
//        p_chemin.push_back(p_destination);
//        return numeric_limits<unsigned int>::max();
//    }
//
    //On a une solution, donc construire le plus court chemin à l'aide de predecesseur[]
//    stack<size_t> pileDuChemin;
//    size_t numero = p_destination;
//    pileDuChemin.push(numero);
//    while (predecesseur[numero] != numeric_limits<size_t>::max())
//    {
//        numero = predecesseur[numero];
//        pileDuChemin.push(numero);
//    }
//    while (!pileDuChemin.empty())
//    {
//        size_t temp = pileDuChemin.top();
//        p_chemin.push_back(temp);
//        pileDuChemin.pop();
//    }
//    return distance[p_destination];
//}


