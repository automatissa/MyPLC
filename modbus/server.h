#pragma once
#include <cstdint>

// ============================================================================
//  Modbus TCP Server — démarre automatiquement au lancement du runtime.
//
//  Toutes les PLC_VAR sont mappées en holding registers automatiquement.
//  La table d'adresses (4xxxx) est affichée à chaque démarrage.
//  Aucun code utilisateur requis — le runtime s'en charge.
//
//  Port 502 — nécessite sudo sur Linux : sudo make run
//
//  Fonction codes supportés :
//    FC03  Read Holding Registers
//    FC06  Write Single Register
//    FC16  Write Multiple Registers
// ============================================================================

namespace myplc {

class ModbusTcpServer {
public:
    // Démarre le serveur en arrière-plan et affiche la table d'adresses.
    void start(uint16_t port = 502);

    // Affiche uniquement la table variable → adresse Modbus.
    void print_map() const;
};

}  // namespace myplc
