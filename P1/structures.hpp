#pragma once
// Structures mémoires pour une collection de groupes musicaux.

#include <string>

struct Groupe; struct Musicien; // Permet d'utiliser les types alors qu'ils seront définis après.

struct ListeGroupes {
	int capacite, nElements;
	Groupe** elements; // Pointeur vers un tableau de Groupe*, chaque Groupe* pointant vers un Groupe.
};

struct ListeMusiciens {
	int capacite, nElements;
	Musicien** elements; // Pointeur vers un tableau de Musicien*, chaque Musicien* pointant vers un Musicien.
};

struct Groupe
{
	std::string nom, genre; // Nom du groupe et genre de musique qu'il fait principalement.
	int anneeFormation; // Année où le groupe a été formé initialement.
	ListeMusiciens membres;
};

struct Musicien
{
	std::string nom, pays; int anneeNaissance;
	ListeGroupes joueDans;
};
