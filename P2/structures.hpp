/*
 * Fichier: structures.hpp
 * Auteurs: Majid Khauly (2434522), Mohamed Elbahrawy (2336883)
 * Description: Définition des classes représentant une collection de groupes musicaux et leurs musiciens.
 *              Contient des listes dynamiques minimalistes (sans std::vector) pour illustrer la gestion manuelle.
 * Date: 23 septembre 2025
 */

#pragma once
// Structures mémoires pour une collection de groupes musicaux.
// Version convertie de struct -> class :
// - Données privées
// - Méthodes (accesseurs) publiques
// NOTE: Le code existant qui faisait objet.champ devra être ajusté en objet.champ() pour utiliser les accesseurs.

#include <string>

class Groupe; class Musicien; // Déclarations anticipées.

class ListeGroupes {
public:
	ListeGroupes() = default;
	ListeGroupes(int capacite, int nElements, Groupe** elements)
		: capacite_(capacite), nElements_(nElements), elements_(elements) {}

	// Accesseurs modifiables
	int& capacite() { return capacite_; }
	int& nElements() { return nElements_; }
	Groupe**& elements() { return elements_; }
	// Accesseurs const
	const int& capacite() const { return capacite_; }
	const int& nElements() const { return nElements_; }
	Groupe** elements() const { return elements_; }

private:
	int capacite_ = 0;
	int nElements_ = 0;
	Groupe** elements_ = nullptr;
};

class ListeMusiciens {
public:
	ListeMusiciens() = default;
	ListeMusiciens(int capacite, int nElements, Musicien** elements)
		: capacite_(capacite), nElements_(nElements), elements_(elements) {}

	int& capacite() { return capacite_; }
	int& nElements() { return nElements_; }
	Musicien**& elements() { return elements_; }
	const int& capacite() const { return capacite_; }
	const int& nElements() const { return nElements_; }
	Musicien** elements() const { return elements_; }

private:
	int capacite_ = 0;
	int nElements_ = 0;
	Musicien** elements_ = nullptr;
};

class Groupe {
public:
	Groupe() = default;
	Groupe(std::string nom, std::string genre, int anneeFormation, const ListeMusiciens& membres)
		: nom_(std::move(nom)), genre_(std::move(genre)), anneeFormation_(anneeFormation), membres_(membres) {}

	std::string& nom() { return nom_; }
	std::string& genre() { return genre_; }
	int& anneeFormation() { return anneeFormation_; }
	ListeMusiciens& membres() { return membres_; }
	const std::string& nom() const { return nom_; }
	const std::string& genre() const { return genre_; }
	const int& anneeFormation() const { return anneeFormation_; }
	const ListeMusiciens& membres() const { return membres_; }

private:
	std::string nom_;
	std::string genre_;
	int anneeFormation_ = 0;
	ListeMusiciens membres_;
};

class Musicien {
public:
	Musicien() = default;
	Musicien(std::string nom, std::string pays, int anneeNaissance, const ListeGroupes& joueDans)
		: nom_(std::move(nom)), pays_(std::move(pays)), anneeNaissance_(anneeNaissance), joueDans_(joueDans) {}

	std::string& nom() { return nom_; }
	std::string& pays() { return pays_; }
	int& anneeNaissance() { return anneeNaissance_; }
	ListeGroupes& joueDans() { return joueDans_; }
	const std::string& nom() const { return nom_; }
	const std::string& pays() const { return pays_; }
	const int& anneeNaissance() const { return anneeNaissance_; }
	const ListeGroupes& joueDans() const { return joueDans_; }

private:
	std::string nom_;
	std::string pays_;
	int anneeNaissance_ = 0;
	ListeGroupes joueDans_;
};
