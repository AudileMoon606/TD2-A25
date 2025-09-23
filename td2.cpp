/*

Noms des auteurs: Majid Khauly (2434522) et Mohamed Elbahrawy (2336883)

Description du programme: 
- Ce programme gère une collection de groupes musicaux et leurs musiciens associés. 
- Il permet de : 
	1. Lire les données à partir d'un fichier binaire
	2. Afficher les groupes et les musiciens
	3. Modifier les informations des musiciens
	4. Gérer la mémoire allouée pour ces structures de données

Date de la remise : Avant 23h30 le dimanche 28 septembre 2025.

*/

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
#include "debogage_memoire.hpp"        // Ajout des numéros de ligne des "new" dans le rapport de fuites.  Doit être après les include du système, qui peuvent utiliser des "placement new" (non supporté par notre ajout de numéros de lignes).

using namespace std;
using namespace iter;

// Constantes (pas de variables globales). Évite les nombres magiques (règle 62) et nommage explicite.
constexpr int  kCapaciteInitialeListe              = 1;  // Capacité minimale d'une liste dynamique.
constexpr int  kFacteurAgrandissementCapacite      = 2;  // Facteur de croissance lors de la réallocation.
constexpr char kLigneSeparationListeGroupes[]      = "\n\033[36m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n";
constexpr char kLigneSeparationPrincipale[]        = "\n\033[35m════════════════════════════════════════\033[0m\n";
constexpr int  kIndicePremierElement               = 0;  // Indice du premier élément d'une liste.

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


void ajouterMusicien(ListeMusiciens& liste, Musicien* musicien)
{
	if (liste.nElements() >= liste.capacite()) {
		int nouvelleCapacite = max(kCapaciteInitialeListe, liste.capacite() * kFacteurAgrandissementCapacite);
		Musicien** nouveauTableau = new Musicien*[nouvelleCapacite];
		for (int i = 0; i < liste.nElements(); i++) {
			nouveauTableau[i] = liste.elements()[i];
		}
		delete[] liste.elements();
		liste.elements() = nouveauTableau;
		liste.capacite() = nouvelleCapacite;
	}
	liste.elements()[liste.nElements()++] = musicien;
}


void ajouterGroupe(ListeGroupes& liste, Groupe* groupe)
{
	if (liste.nElements() >= liste.capacite()) {
		int nouvelleCapacite = max(kCapaciteInitialeListe, liste.capacite() * kFacteurAgrandissementCapacite);
		Groupe** nouveauTableau = new Groupe*[nouvelleCapacite];
		for (int i = 0; i < liste.nElements(); i++) {
			nouveauTableau[i] = liste.elements()[i];
		}
		delete[] liste.elements();
		liste.elements() = nouveauTableau;
		liste.capacite() = nouvelleCapacite;
	}
	liste.elements()[liste.nElements()++] = groupe;
}

void enleverGroupe(ListeGroupes& liste, Groupe* groupe)
{
	for (int i = 0; i < liste.nElements(); i++) {
		if (liste.elements()[i] == groupe) {
			// Remplacer par le dernier élément pour éviter de décaler
			liste.elements()[i] = liste.elements()[liste.nElements() - 1];
			liste.nElements()--;
			break;
		}
	}
}

Musicien* trouverMusicien(const ListeGroupes& listeGroupes, const string& nom)
{
    for (int i = 0; i < listeGroupes.nElements(); i++) {
        Groupe* groupe = listeGroupes.elements()[i];
        for (int j = 0; j < groupe->membres().nElements(); j++) {
            Musicien* musicien = groupe->membres().elements()[j];
            if (musicien->nom() == nom) {
                return musicien;
            }
        }
    }
    return nullptr;
}

Musicien* lireMusicien(istream& fichier, const ListeGroupes& listeGroupes)
{
    // Lecture des champs (initialisation immédiate plutôt que d'allouer puis assigner)
    string nom            = lireString(fichier);
    string pays           = lireString(fichier);
    int    anneeNaissance = lireUint16(fichier);

    // Chercher si le musicien existe déjà (évite duplication)
    Musicien* musicienExistant = trouverMusicien(listeGroupes, nom);
	if (musicienExistant != nullptr) {
        return musicienExistant;
    }

    // Créer un nouveau musicien directement avec les valeurs lues
    Musicien* nouveauMusicien = new Musicien(nom, pays, anneeNaissance, ListeGroupes{});
    cout << "Création du musicien: " << nouveauMusicien->nom() << endl;
    return nouveauMusicien;
}

Groupe* lireGroupe(istream& fichier, ListeGroupes& listeGroupes)
{
	Groupe* groupe = new Groupe{};
	groupe->nom()   = lireString(fichier);
	groupe->genre() = lireString(fichier);
	groupe->anneeFormation() = lireUint16 (fichier);
	groupe->membres().nElements() = lireUint8 (fichier);  // allocation directe possible

	// Allouer le tableau des membres
	groupe->membres().capacite() = groupe->membres().nElements();
	groupe->membres().elements() = new Musicien*[groupe->membres().capacite()];

	cout << groupe->nom() << endl;
	for (int i = 0; i < groupe->membres().nElements(); i++) {
		Musicien* musicien = lireMusicien(fichier, listeGroupes);
		groupe->membres().elements()[i] = musicien;
		ajouterGroupe(musicien->joueDans(), groupe);
	}
	return groupe;
}

ListeGroupes creerListe(string nomFichier)
{
	ifstream fichier(nomFichier, ios::binary);
	fichier.exceptions(ios::failbit);

	int nGroupes = lireUint16(fichier);

	ListeGroupes listeGroupes; // défaut (0,0,nullptr)
	for (int i = 0; i < nGroupes; i++) {
		Groupe* groupe = lireGroupe(fichier, listeGroupes);
		ajouterGroupe(listeGroupes, groupe);
	}

	return listeGroupes;
}

void detruireGroupe(Groupe* groupe)
{
	// Enlever ce groupe de la liste des groupes de chaque membre
	for (int i = 0; i < groupe->membres().nElements(); i++) {
		Musicien* musicien = groupe->membres().elements()[i];
		enleverGroupe(musicien->joueDans(), groupe);
		// Si le musicien ne joue plus dans aucun groupe, le détruire
		if (musicien->joueDans().nElements() == 0) {
			cout << "Destruction du musicien: " << musicien->nom() << endl;
			delete[] musicien->joueDans().elements();
			delete musicien;
		}
	}
	// Libérer la mémoire du tableau des membres et du groupe
	delete[] groupe->membres().elements();
	delete groupe;
}

void detruireListeGroupes(ListeGroupes& listeGroupes)
{
	for (int i = 0; i < listeGroupes.nElements(); i++) {
		detruireGroupe(listeGroupes.elements()[i]);
	}
	delete[] listeGroupes.elements();
	listeGroupes.elements() = nullptr;
	listeGroupes.nElements() = 0;
	listeGroupes.capacite() = 0;
}

void afficherMusicien(const Musicien& musicien)
{
	cout << "  " << musicien.nom() << ", " << musicien.pays() << ", " << musicien.anneeNaissance() << endl;
}

void afficherGroupe(const Groupe& groupe)
{
	cout << groupe.nom() << ", " << groupe.genre() << ", " << groupe.anneeFormation() << endl;
	for (int i = 0; i < groupe.membres().nElements(); i++) {
		afficherMusicien(*groupe.membres().elements()[i]);
	}
}

void afficherListeGroupes(const ListeGroupes& listeGroupes)
{
	cout << kLigneSeparationListeGroupes;
	span<Groupe*> groupes(listeGroupes.elements(), static_cast<size_t>(listeGroupes.nElements()));
	for (Groupe* groupe : groupes) {
		afficherGroupe(*groupe);
		cout << kLigneSeparationListeGroupes;
	}
}

void afficherGroupesMusicien(const ListeGroupes& listeGroupes, const string& nomMusicien)
{
	const Musicien* musicien = trouverMusicien(listeGroupes, nomMusicien);
	if (musicien == nullptr) {
        cout << "Aucun musicien de ce nom" << endl;
    } else {
        afficherListeGroupes(musicien->joueDans());
    }
}

int main()
{
	bibliotheque_cours::activerCouleursAnsi();  // Permet sous Windows les "ANSI escape code" pour changer de couleurs https://en.wikipedia.org/wiki/ANSI_escape_code ; les consoles Linux/Mac les supportent normalement par défaut.

	//int* fuite = new int;

	ListeGroupes listeGroupes = creerListe("groupes.bin");

	cout << kLigneSeparationPrincipale << "Le premier groupe de la liste est:" << endl;
	afficherGroupe(*listeGroupes.elements()[kIndicePremierElement]);

	cout << kLigneSeparationPrincipale << "Les groupes sont:" << endl;
	afficherListeGroupes(listeGroupes);

	Musicien* daveGrohl = trouverMusicien(listeGroupes, "Dave Grohl");
	if (daveGrohl != nullptr) {
		daveGrohl->pays() = "États-Unis";
	}

	cout << kLigneSeparationPrincipale << "Liste des groupes où Dave Grohl joue sont:" << endl;
	afficherGroupesMusicien(listeGroupes, "Dave Grohl");

	Groupe* premierGroupe = listeGroupes.elements()[kIndicePremierElement];
	enleverGroupe(listeGroupes, premierGroupe);
	detruireGroupe(premierGroupe);

	cout << kLigneSeparationPrincipale << "Les groupes sont maintenant:" << endl;
	afficherListeGroupes(listeGroupes);

	detruireListeGroupes(listeGroupes);
}
