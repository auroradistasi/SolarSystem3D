## Compilazione:
```bash
cmake -B build -D CMAKE_BUILD_TYPE=Release
cmake --build build
```

## Esecuzione:
```bash
cd build

./tappa01
./tappa02
./tappa03
./tappa04
./tappa05
./tappa06
./tappa07
```
Le tappe non richiedono argomenti aggiuntivi da riga di comando. 

## Comandi interfaccia utente:

### Mouse
| Comando | Azione | Da |
|---|---|---|
| Trascinamento con tasto sinistro | Ruota la camera | Tappa02 |
| `Ctrl` + mouse su/giù | Avvicina o allontana la camera | Tappa02 |

### Tastiera
| Comando | Azione | Da |
|---|---|---|
| `0` | Camera sul Sole, vista panoramica | Tappa04 |
| `1` … `7` | Camera su un pianeta, da Mercurio a Urano | Tappa04 |
| `8` | Camera su Luna | Tappa07 |
| `9` | Camera su Nettuno | Tappa07 |
| `P` | Pausa / riprendi | Tappa04 |
| `↑` / `↓` | Accelera / rallenta il tempo | Tappa04 |
| `R` | Velocità del tempo a 1× | Tappa04 |
| `O` | Mostra / nascondi le orbite | Tappa04 |

La Tappa01 non ha comandi interattivi. La Tappa03, la Tappa05 e la Tappa06 non aggiungono comandi. 
La finestra è ridimensionabile in tutte le tappe.