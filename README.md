# Projekt z Systemów Operacyjnych - Temat 8 Basen
[Link do repozytorium](https://github.com/HinoYoseii/basen)
---

#### **Cel projektu**
Celem projektu jest stworzenie symulacji systemu zarządzania kompleksem basenów krytych, który uwzględnia regulamin korzystania z basenów, różne typy użytkowników (dzieci, dorośli, VIP), oraz sytuacje awaryjne związane z bezpieczeństwem i wymianą wody.

---

### **Dostępne baseny**
- **Basen olimpijski**: 
  - Tylko dla osób pełnoletnich.
  - Maksymalna liczba osób: **X1**.
- **Basen rekreacyjny**:
  - Średnia wieku użytkowników nie może przekroczyć 40 lat.
  - Maksymalna liczba osób: **X2**.
- **Brodzik dla dzieci**:
  - Dostępny dla dzieci do 5 roku życia i ich opiekunów.
  - Maksymalna liczba osób: **X3**.

---

### **Korzystanie z kompleksu**:
  - Kompleks jest otwarty w godzinach **Tp**–**Tk**.
  - Klienci kupują bilety czasowe na określony czas **Ti**, pozwalające na korzystanie z dowolnego basenu.

### **Przerwy w korzystaniu z basenów**:
   - Polecenia ratownika:
     - Na sygnał ratownika (sygnał1), wszyscy muszą natychmiast opuścić dany basen.
     - Po zakończeniu sytuacji (sygnał2), klienci mogą wrócić do korzystania z basenu.
   - Okresowa wymiana wody w całym kompleksie:
     - W tym czasie cały obiekt jest zamknięty.
     - Wszyscy użytkownicy muszą opuścić teren pływalni.

---

#### **Regulamin pływalni**
- Tylko osoby pełnoletnie mogą korzystać z basenu olimpijskiego.
- Dzieci poniżej 10 roku życia muszą przebywać pod opieką osoby dorosłej.
- W brodziku mogą kąpać się wyłącznie dzieci do 5 roku życia i ich opiekunowie.
- Średnia wieku w basenie rekreacyjnym nie może przekroczyć 40 lat.
- Dzieci do 3 roku życia muszą pływać w pampersach.
- Noszenie czepków pływackich nie jest obowiązkowe.

---

#### **Elementy do zaimplementowania**
1. **Programy zarządzające:**
   - **Ratownik**:
     - Do każdego basenu przypisany jest jeden ratownik.
     - Obsługuje sygnały awaryjne (sygnał1 i sygnał2).
   - **Kasjer**:
     - Sprzedaje bilety, weryfikuje wiek klientów oraz uprawnienia VIP.
     - Jeżeli klient ma status VIP może wejść na basen pomijając kolejke
     - Jeżeli klient jest poniżej 10 roku życia to nie płaci za bilet
   - **Klienci**:
     - Proces klienta reprezentuje zachowanie jednej osoby lub opiekuna z dzieckiem.
     - Klienci poniżej 10 roku życia muszą przebywać na basenie z osoba dorosłą (jeden proces)
     - Klienci do 3 roku życia muszą mieć pampersy.

2. **Mechanizmy bezpieczeństwa**:
   - Obsługa sygnałów ratowników.
   - Procedura zamykania obiektu na czas wymiany wody.

3. **Symulacja wejść i wyjść klientów**:
   - Losowe pojawianie się klientów w różnych przedziałach wiekowych (1–70 lat).
   - Dynamiczne przypisywanie do odpowiednich basenów zgodnie z regulaminem na określony czas **Ti** .

---
