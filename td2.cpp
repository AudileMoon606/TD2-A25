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
#include "debogage_memoire.hpp"        // Ajout des numéros de ligne des "new" dans le rapport de fuites.  Doit être après les include du système, qui peuvent utiliser des "placement new" (non supporté par notre ajout de numéros de lignes).

using namespace std;
using namespace iter;

#pragma endregion//}

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
	if (liste.nElements >= liste.capacite) {
		int nouvelleCapacite = max(1, liste.capacite * 2);
		Musicien** nouveauTableau = new Musicien*[nouvelleCapacite];
		for (int i = 0; i < liste.nElements; i++) {
			nouveauTableau[i] = liste.elements[i];
		}
		delete[] liste.elements;
		liste.elements = nouveauTableau;
		liste.capacite = nouvelleCapacite;
	}
	liste.elements[liste.nElements++] = musicien;
}


/*Fonction pour ajouter un groupe à une liste, qui fait la réallocation du tableau en doublant sa capacité 
s’il ne reste pas de place en s’assurant qu’il y a au moins une capacité d’un élément (sinon, le double de 
zéro  resterait  zéro).  En  C++  il  n’est  pas  possible  de  changer  la  taille  d’une  allocation,  alors  il  faut 
allouer  un  nouveau  tableau,  copier  de  l’ancien  au  nouveau  et  détruire  l’ancien  tableau  trop  petit.    Le 
groupe à ajouter est déjà alloué, il faut simplement ajouter à la liste le pointeur vers le groupe existant.*/

void ajouterGroupe(ListeGroupes& liste, Groupe* groupe)
{
	if (liste.nElements >= liste.capacite) {
		int nouvelleCapacite = max(1, liste.capacite * 2);
		Groupe** nouveauTableau = new Groupe*[nouvelleCapacite];
		for (int i = 0; i < liste.nElements; i++) {
			nouveauTableau[i] = liste.elements[i];
		}
		delete[] liste.elements;
		liste.elements = nouveauTableau;
		liste.capacite = nouvelleCapacite;
	}
	liste.elements[liste.nElements++] = groupe;
}


/* Fonction pour enlever un groupe d’une liste, qui prend un pointeur vers un groupe et enlève ce groupe 
de  la  liste  sans  détruire  le  groupe.    L’ajout  du  groupe  utilisait  un  groupe  existant,  et  le  groupe  existe 
encore après avoir enlevé le groupe de la collection, ces fonctions sont donc symétriques. Des fonctions 
séparées serviront à créer et détruire les groupes. */

void enleverGroupe(ListeGroupes& liste, Groupe* groupe)
{
	for (int i = 0; i < liste.nElements; i++) {
		if (liste.elements[i] == groupe) {
			// Remplacer par le dernier élément pour éviter de décaler
			liste.elements[i] = liste.elements[liste.nElements - 1];
			liste.nElements--;
			break;
		}
	}
}


/* Fonction  pour  trouver  un  musicien,  qui  cherche  dans  tous  les  groupes  d’une  collection  un  musicien 
par son nom, et retourne un  pointeur  vers  ce musicien (ou nullptr  si le musicien n’est pas trouvé). On 
suppose que le nom d’un musicien l’identifie de manière unique, i.e. si on voit le même nom deux fois, 
c’est le même musicien. */
Musicien* trouverMusicien(const ListeGroupes& listeGroupes, const string& nom)
{
	span<Groupe*> groupes(listeGroupes.elements, listeGroupes.nElements);
	for (Groupe* groupe : groupes) {
		span<Musicien*> membres(groupe->membres.elements, groupe->membres.nElements);
		for (Musicien* musicien : membres) {
			if (musicien->nom == nom) {
				return musicien;
			}
		}
	}
	return nullptr;
}

/* Fonctions pour créer une collection à partir du fichier (creerListe/lireGroupe/lireMusicien), 
qui allouent la capacité nécessaire pour les groupes dans le fichier, qui charge les données de chacun de 
ces groupes; chaque groupe contient une liste de musiciens, qu'il faut aussi allouer, et il faut allouer la 
mémoire pour chaque musicien. Attention : Le fichier contient certains musiciens plus d'une fois, mais 
nous voulons qu'en mémoire l'allocation soit faite une seule fois par musicien différent (on utilisera la 
fonction pour trouver un musicien par nom, pour vérifier si un musicien a déjà été alloué). */

Musicien* lireMusicien(istream& fichier, const ListeGroupes& listeGroupes)
{
	Musicien musicien = {};
	musicien.nom            = lireString(fichier);
	musicien.pays           = lireString(fichier);
	musicien.anneeNaissance = lireUint16(fichier);
	
	// Chercher si le musicien existe déjà
	Musicien* musicienExistant = trouverMusicien(listeGroupes, musicien.nom);
	if (musicienExistant != nullptr) {
		return musicienExistant;
	}
	
	// Créer un nouveau musicien
	Musicien* nouveauMusicien = new Musicien(musicien);
	nouveauMusicien->joueDans = {0, 0, nullptr};
	cout << "Création du musicien: " << nouveauMusicien->nom << endl;
	return nouveauMusicien;
}

Groupe* lireGroupe(istream& fichier, ListeGroupes& listeGroupes)
{
	Groupe* groupe = new Groupe{};
	groupe->nom   = lireString(fichier);
	groupe->genre = lireString(fichier);
	groupe->anneeFormation = lireUint16 (fichier);
	groupe->membres.nElements = lireUint8 (fichier);  //NOTE: Vous avez le droit d'allouer d'un coup le tableau pour les membres, sans faire de réallocation comme pour ListeGroupes.  Vous pouvez aussi copier-coller les fonctions d'allocation de ListeGroupes ci-dessus dans des nouvelles fonctions et faire un remplacement de Groupe par Musicien, pour réutiliser cette réallocation.
	
	// Allouer le tableau des membres
	groupe->membres.capacite = groupe->membres.nElements;
	groupe->membres.elements = new Musicien*[groupe->membres.capacite];
	
	cout << groupe->nom << endl;
	for (int i = 0; i < groupe->membres.nElements; i++) {
		Musicien* musicien = lireMusicien(fichier, listeGroupes);
		groupe->membres.elements[i] = musicien;
		ajouterGroupe(musicien->joueDans, groupe);
	}
	return groupe;
}

ListeGroupes creerListe(string nomFichier)
{
	ifstream fichier(nomFichier, ios::binary);
	fichier.exceptions(ios::failbit);
	
	int nElements = lireUint16(fichier);

	ListeGroupes listeGroupes = {0, 0, nullptr};
	for (int i = 0; i < nElements; i++) {
		Groupe* groupe = lireGroupe(fichier, listeGroupes);
		ajouterGroupe(listeGroupes, groupe);
	}
	
	return listeGroupes;
}

void detruireGroupe(Groupe* groupe)
{
	// Enlever ce groupe de la liste des groupes de chaque membre
	for (int i = 0; i < groupe->membres.nElements; i++) {
		Musicien* musicien = groupe->membres.elements[i];
		enleverGroupe(musicien->joueDans, groupe);
		
		// Si le musicien ne joue plus dans aucun groupe, le détruire
		if (musicien->joueDans.nElements == 0) {
			cout << "Destruction du musicien: " << musicien->nom << endl;
			delete[] musicien->joueDans.elements;
			delete musicien;
		}
	}
	
	// Libérer la mémoire du tableau des membres et du groupe
	delete[] groupe->membres.elements;
	delete groupe;
}

void detruireListeGroupes(ListeGroupes& listeGroupes)
{
	for (int i = 0; i < listeGroupes.nElements; i++) {
		detruireGroupe(listeGroupes.elements[i]);
	}
	delete[] listeGroupes.elements;
	listeGroupes.elements = nullptr;
	listeGroupes.nElements = 0;
	listeGroupes.capacite = 0;
}

void afficherMusicien(const Musicien& musicien)
{
	cout << "  " << musicien.nom << ", " << musicien.pays << ", " << musicien.anneeNaissance << endl;
}

void afficherGroupe(const Groupe& groupe)
{
	cout << groupe.nom << ", " << groupe.genre << ", " << groupe.anneeFormation << endl;
	for (int i = 0; i < groupe.membres.nElements; i++) {
		afficherMusicien(*groupe.membres.elements[i]);
	}
}

void afficherListeGroupes(const ListeGroupes& listeGroupes)
{
	static const string ligneDeSeparation = "\n\033[36m▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒▒\033[0m\n";
	cout << ligneDeSeparation;
	span<Groupe*> groupes(listeGroupes.elements, listeGroupes.nElements);
	for (Groupe* groupe : groupes) {
		afficherGroupe(*groupe);
		cout << ligneDeSeparation;
	}
}

void afficherGroupesMusicien(const ListeGroupes& listeGroupes, const string& nomMusicien)
{
	const Musicien* musicien = trouverMusicien(listeGroupes, nomMusicien);
	if (musicien == nullptr)
		cout << "Aucun musicien de ce nom" << endl;
	else
		afficherListeGroupes(musicien->joueDans);
}

int main()
{
	bibliotheque_cours::activerCouleursAnsi();  // Permet sous Windows les "ANSI escape code" pour changer de couleurs https://en.wikipedia.org/wiki/ANSI_escape_code ; les consoles Linux/Mac les supportent normalement par défaut.

	//int* fuite = new int;

	static const string ligneDeSeparation = "\n\033[35m════════════════════════════════════════\033[0m\n";

	ListeGroupes listeGroupes = creerListe("groupes.bin");
	
	cout << ligneDeSeparation << "Le premier groupe de la liste est:" << endl;
	afficherGroupe(*listeGroupes.elements[0]);
	
	cout << ligneDeSeparation << "Les groupes sont:" << endl;
	afficherListeGroupes(listeGroupes);
	
	Musicien* daveGrohl = trouverMusicien(listeGroupes, "Dave Grohl");
	if (daveGrohl != nullptr) {
		daveGrohl->pays = "États-Unis";
	}
	
	cout << ligneDeSeparation << "Liste des groupes où Dave Grohl joue sont:" << endl;
	afficherGroupesMusicien(listeGroupes, "Dave Grohl");
	
	Groupe* premierGroupe = listeGroupes.elements[0];
	enleverGroupe(listeGroupes, premierGroupe);
	detruireGroupe(premierGroupe);
	
	cout << ligneDeSeparation << "Les groupes sont maintenant:" << endl;
	afficherListeGroupes(listeGroupes);
	
	detruireListeGroupes(listeGroupes);
}
