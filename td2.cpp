//=================================================================================\n// Fichier adapté pour revenir au format du gabarit original (td2 original.cpp)\n// tout en conservant l'implémentation fonctionnelle réalisée.\n// Auteurs: Majid Khauly (2434522) et Mohamed Elbahrawy (2336883)\n//=================================================================================

#pragma region "Includes"//{
#define _CRT_SECURE_NO_WARNINGS // On permet d'utiliser les fonctions de copies de chaînes qui sont considérées non sécuritaires.

#include "structures.hpp"      // Structures de données pour la collection de groupes musicaux en mémoire.

#include <iostream>
#include <fstream>
#include <string>
#include <limits>
#include <algorithm>
#include <span>

#include "cppitertools/range.hpp"

#include "bibliotheque_cours.hpp"
#include "verification_allocation.hpp" // Nos fonctions pour le rapport de fuites de mémoire.
#include "debogage_memoire.hpp"        // Ajout des numéros de ligne des "new" dans le rapport de fuites.  Doit être après les include du système.

using namespace std;
using namespace iter;
#pragma endregion//}

// Constantes internes (simples, style gabarit : pas de préfixe k_ global imposé dans le gabarit, mais conservées localement).
static constexpr int CAPACITE_INITIALE = 1;
static constexpr int FACTEUR_AGRANDISSEMENT = 2;
static constexpr int INDICE_PREMIER = 0;
static const char* LIGNE_SEPARATION_LISTE = "\n\033[36m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n";
static const char* LIGNE_SEPARATION_PRINCIPALE = "\n\033[35m════════════════════════════════════════\033[0m\n";

typedef uint8_t UInt8;
typedef uint16_t UInt16;

#pragma region "Fonctions de base pour lire le fichier binaire"//{

UInt8 lireUint8(istream& fichier)
{
	UInt8 valeur = 0;
	fichier.read((char*)&valeur, sizeof(valeur));
	return valeur;
}
UInt16 lireUint16(istream& fichier)
{
	UInt16 valeur = 0;
	fichier.read((char*)&valeur, sizeof(valeur));
	return valeur;
}
string lireString(istream& fichier)
{
	string texte;
	texte.resize(lireUint16(fichier));
	fichier.read((char*)&texte[0], streamsize(sizeof(texte[0])) * texte.length());
	return texte;
}

#pragma endregion//}


//=================================================================================
// Fonctions implantant les TODO du gabarit original.
//=================================================================================

// Ajoute un musicien (tableau dynamique de pointeurs, sans copier l'objet Musicien).
void ajouterMusicien(ListeMusiciens& liste, Musicien* musicien)
{
	if (liste.nElements() >= liste.capacite()) {
		int nouvelleCapacite = max(CAPACITE_INITIALE, liste.capacite() * FACTEUR_AGRANDISSEMENT);
		Musicien** nouveauTableau = new Musicien*[nouvelleCapacite];
		for (int i : range(liste.nElements()))
			nouveauTableau[i] = liste.elements()[i];
		delete[] liste.elements();
		liste.elements() = nouveauTableau;
		liste.capacite() = nouvelleCapacite;
	}
	liste.elements()[liste.nElements()++] = musicien;
}

// Ajoute un groupe (même logique que pour ajouterMusicien).
void ajouterGroupe(ListeGroupes& liste, Groupe* groupe)
{
	if (liste.nElements() >= liste.capacite()) {
		int nouvelleCapacite = max(CAPACITE_INITIALE, liste.capacite() * FACTEUR_AGRANDISSEMENT);
		Groupe** nouveauTableau = new Groupe*[nouvelleCapacite];
		for (int i : range(liste.nElements()))
			nouveauTableau[i] = liste.elements()[i];
		delete[] liste.elements();
		liste.elements() = nouveauTableau;
		liste.capacite() = nouvelleCapacite;
	}
	liste.elements()[liste.nElements()++] = groupe;
}

// Enlève un groupe (ne détruit pas l'objet pointé). Ordre non conservé.
void enleverGroupe(ListeGroupes& liste, Groupe* groupe)
{
	for (int i : range(liste.nElements())) {
		if (liste.elements()[i] == groupe) {
			liste.elements()[i] = liste.elements()[liste.nElements() - 1];
			liste.nElements()--; break;
		}
	}
}

// Trouve un musicien par nom via un parcours des groupes (utilise span pour illustrer le gabarit).
Musicien* trouverMusicien(const ListeGroupes& listeGroupes, const string& nom)
{
	span<Groupe*> groupes(listeGroupes.elements(), static_cast<size_t>(listeGroupes.nElements()));
	for (Groupe* g : groupes) {
		span<Musicien*> membres(g->membres().elements(), static_cast<size_t>(g->membres().nElements()));
		for (Musicien* m : membres) {
			if (m->nom() == nom)
				return m;
		}
	}
	return nullptr;
}

// Lecture d'un musicien : retourne un pointeur vers un existant (si trouvé) ou un nouveau.
Musicien* lireMusicien(istream& fichier, const ListeGroupes& listeGroupes)
{
	string nom = lireString(fichier);
	string pays = lireString(fichier);
	int anneeNaissance = lireUint16(fichier);
	if (Musicien* existant = trouverMusicien(listeGroupes, nom))
		return existant;
	Musicien* nouveau = new Musicien(nom, pays, anneeNaissance, ListeGroupes{});
	cout << "Création du musicien: " << nouveau->nom() << endl; // Debug (pas de doublons attendus)
	return nouveau;
}

// Lecture d'un groupe; lie les pointeurs musiciens et mise à jour des groupes où ils jouent.
Groupe* lireGroupe(istream& fichier, ListeGroupes& listeGroupes)
{
	Groupe* groupe = new Groupe{};
	groupe->nom() = lireString(fichier);
	groupe->genre() = lireString(fichier);
	groupe->anneeFormation() = lireUint16(fichier);
	groupe->membres().nElements() = lireUint8(fichier);
	groupe->membres().capacite() = groupe->membres().nElements();
	groupe->membres().elements() = new Musicien*[groupe->membres().capacite()];
	cout << groupe->nom() << endl; // Pour suivre la lecture des groupes
	for (int i : range(groupe->membres().nElements())) {
		Musicien* m = lireMusicien(fichier, listeGroupes);
		groupe->membres().elements()[i] = m;
		ajouterGroupe(m->joueDans(), groupe);
	}
	return groupe;
}

// Crée la liste de groupes complète à partir du fichier.
ListeGroupes creerListe(string nomFichier)
{
	ifstream fichier(nomFichier, ios::binary); fichier.exceptions(ios::failbit);
	int nGroupes = lireUint16(fichier);
	ListeGroupes liste; // vide (0,0,nullptr)
	for (int i : range(nGroupes)) {
		Groupe* g = lireGroupe(fichier, liste);
		ajouterGroupe(liste, g);
	}
	return liste;
}

// Détruit un groupe et supprime les musiciens devenus orphelins.
void detruireGroupe(Groupe* groupe)
{
	for (int i : range(groupe->membres().nElements())) {
		Musicien* m = groupe->membres().elements()[i];
		enleverGroupe(m->joueDans(), groupe);
		if (m->joueDans().nElements() == 0) { // Orphelin
			cout << "Destruction du musicien: " << m->nom() << endl;
			delete[] m->joueDans().elements();
			delete m;
		}
	}
	delete[] groupe->membres().elements();
	delete groupe;
}

// Détruit toute la liste et son contenu.
void detruireListeGroupes(ListeGroupes& liste)
{
	for (int i : range(liste.nElements()))
		detruireGroupe(liste.elements()[i]);
	delete[] liste.elements();
	liste.elements() = nullptr;
	liste.nElements() = 0;
	liste.capacite() = 0;
}

// Affiche un musicien.
void afficherMusicien(const Musicien& m)
{ cout << "  " << m.nom() << ", " << m.pays() << ", " << m.anneeNaissance() << endl; }

// Affiche un groupe et ses membres.
void afficherGroupe(const Groupe& g)
{
	cout << g.nom() << ", " << g.genre() << ", " << g.anneeFormation() << endl;
	for (int i : range(g.membres().nElements()))
		afficherMusicien(*g.membres().elements()[i]);
}

// Affiche une liste de groupes (utilise une ligne Unicode, style gabarit).
void afficherListeGroupes(const ListeGroupes& liste)
{
	cout << LIGNE_SEPARATION_LISTE;
	span<Groupe*> groupes(liste.elements(), static_cast<size_t>(liste.nElements()));
	for (Groupe* g : groupes) {
		afficherGroupe(*g);
		cout << LIGNE_SEPARATION_LISTE;
	}
}

// Affiche les groupes d'un musicien trouvé par son nom.
void afficherGroupesMusicien(const ListeGroupes& liste, const string& nomMusicien)
{
	if (const Musicien* m = trouverMusicien(liste, nomMusicien))
		afficherListeGroupes(m->joueDans());
	else
		cout << "Aucun musicien de ce nom" << endl;
}

int main()
{
	bibliotheque_cours::activerCouleursAnsi();

	// Lecture et création de toute la structure (affiche les noms des 22 musiciens sans doublons).
	ListeGroupes listeGroupes = creerListe("groupes.bin");

	cout << LIGNE_SEPARATION_PRINCIPALE << "Le premier groupe de la liste est:" << endl;
	afficherGroupe(*listeGroupes.elements()[INDICE_PREMIER]); // Devrait être Nirvana

	cout << LIGNE_SEPARATION_PRINCIPALE << "Les groupes sont:" << endl;
	afficherListeGroupes(listeGroupes); // 7 groupes

	// Modification pays de Dave Grohl.
	if (Musicien* dave = trouverMusicien(listeGroupes, "Dave Grohl"))
		dave->pays() = "États-Unis";

	cout << LIGNE_SEPARATION_PRINCIPALE << "Liste des groupes où Dave Grohl joue sont:" << endl;
	afficherGroupesMusicien(listeGroupes, "Dave Grohl"); // Nirvana + Foo Fighters

	// Destruction du premier groupe (Nirvana) et conséquences sur musiciens.
	Groupe* premier = listeGroupes.elements()[INDICE_PREMIER];
	enleverGroupe(listeGroupes, premier);
	detruireGroupe(premier); // Détruit Kurt Cobain & Krist Novoselic si orphelins.

	cout << LIGNE_SEPARATION_PRINCIPALE << "Les groupes sont maintenant:" << endl;
	afficherListeGroupes(listeGroupes);

	// Nettoyage complet.
	detruireListeGroupes(listeGroupes);
}
