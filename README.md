## Projekt: Symulator Windy (C++ + GDI+ / WinAPI)

## Opis
Ten projekt to graficzny symulator windy napisany w języku C++ z użyciem biblioteki WinAPI oraz GDI+. Winda obsługuje piętra 1–5, zabiera i wysadza pasażerów zgodnie z ustalonymi zasadami działania.

## Funkcje

- Obsługa wielu pasażerów w jednym kursie (w tym samym kierunku).
- Dynamiczne podejmowanie decyzji o kierunku jazdy.
- Graficzna reprezentacja windy i pasażerów na piętrach.
- Stany windy:
  - BEZCZYNNOSC
  - JEDZIE_DO_OSOBY
  - CZEKA_NA_OSOBE
  - JEDZIE_DO_CELU
  - CZEKA_PO_WYSADZENIU
- Logika przewozu: winda nie wraca od razu na 1. piętro, jeśli są jeszcze osoby czekające w tym samym kierunku.
- W przypadku braku zgłoszeń, winda po 5 sekundach zjeżdża na 1 piętro.

## Uruchomienie

1. Otwórz projekt w Visual Studio.
2. Skonfiguruj linker, jeśli to konieczne (dodaj `gdiplus.lib`).
3. Upewnij się, że system wspiera GDI+.
4. Naciśnij `F5`, by uruchomić program.

## Obsługa

- Kliknięcie przycisku piętra dodaje pasażera na to piętro.
- Każdy pasażer ma:
  - Piętro początkowe.
  - Piętro docelowe.
- Winda wybiera optymalną trasę (priorytet pasażerów jadących w tym samym kierunku).
- Po wysadzeniu ostatniego pasażera i braku zgłoszeń — winda po 5 sekundach wraca na 1. piętro.

## 📋 Wymagania

- Windows (WinAPI + GDI+)
- Visual Studio (zalecane: 2019 lub 2022)
- `gdiplus.h`, `windows.h`

## Struktura

- `main.cpp` – funkcja główna i pętla wiadomości
- `Winda.h/.cpp` – logika windy
- `Rysowanie.h/.cpp` – funkcje GDI+ do rysowania
- `Passenger.h` – struktura osoby czekającej
- `enum.h` – definicje stanów windy

## Autorki

203383 Alicja Szajgin
203721 Amelia Lipińska

