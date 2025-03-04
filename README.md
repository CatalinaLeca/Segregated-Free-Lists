# **Segregated Free Lists**
Autor: **Maria-Catalina Leca - 315CA**  
Copyright © 2023-2024  
Materia: **Structuri de Date**  
Tema 1 - **Segregated Free Lists (Alocator de Memorie în C)**  

---

## 📖 **Descrierea Proiectului**
Acest proiect implementează un **allocator de memorie** folosind **Segregated Free Lists**, o tehnică eficientă pentru gestionarea blocurilor de memorie de dimensiuni variabile. Programul oferă funcționalități similare celor din bibliotecile standard (`malloc()`, `free()`) și permite alocarea, eliberarea și manipularea memoriei într-un mod optimizat.

### 🔹 **Obiective**
- **Aprofundarea utilizării limbajului C**, cu accent pe gestionarea memoriei dinamice.
- **Implementarea și utilizarea listelor dublu înlănțuite**, esențiale pentru gestionarea memoriei libere.
- **Familiarizarea cu structurile de date generice**, utilizând `void*` pentru a permite gestionarea eficientă a diferitelor tipuri de date.

---

## ⚙️ **Funcționalități principale**
Programul implementează un set de comenzi pentru alocarea și gestionarea memoriei:

### 🔹 **Inițializarea și gestionarea heap-ului**
- `INIT_HEAP <adresă_start_heap> <număr_liste> <număr_bytes_per_listă> <tip_reconstituire>`  
  - Creează structura de **Segregated Free Lists**, organizând blocurile de memorie în liste dublu înlănțuite.
  - Fiecare listă conține blocuri de dimensiuni **puteri ale lui 2**, începând cu `8 bytes`.
  - Se gestionează memoria printr-un **heap prealocat**, cu opțiunea de **reconstituire a fragmentelor**.

- `DESTROY_HEAP`  
  - Eliberează toate resursele și închide programul.

### 🔹 **Alocare și eliberare memorie**
- `MALLOC <nr_bytes>`  
  - Alocă un bloc de **cel puțin** `nr_bytes` din cea mai mică listă disponibilă.
  - Dacă blocul este mai mare decât necesar, **se fragmentează**, restul fiind returnat în lista de blocuri libere.
  - Dacă nu există blocuri suficient de mari, se afișează eroarea **"Out of memory"**.

- `FREE <adresă>`  
  - Marchează blocul de la `adresă` ca fiind liber.
  - Dacă politica de reconstituire (`tip_reconstituire`) este `1`, încearcă să **recombine** blocurile adiacente pentru a forma dimensiunea originală.
  - Dacă adresa nu este validă sau blocul nu a fost alocat, se afișează **"Invalid free"**.

### 🔹 **Citire și scriere în memorie**
- `WRITE <adresă> <date> <nr_bytes>`  
  - Scrie `nr_bytes` de la adresa specificată, verificând limitele memoriei.
  - Dacă zona nu a fost alocată, se afișează eroarea **"Segmentation fault (core dumped)"**.

- `READ <adresă> <nr_bytes>`  
  - Afișează conținutul memoriei alocate, dacă zona este validă.
  - Dacă nu este, se afișează eroarea **"Segmentation fault (core dumped)"**.

### 🔹 **Diagnostic și analiză memorie**
- `DUMP_MEMORY`  
  - Afișează statisticile memoriei:
    - Memorie totală
    - Memorie alocată și liberă
    - Numărul de blocuri libere/alocate
    - Apeluri `MALLOC`, `FREE` și **numărul de fragmentări**.
  - Listează blocurile de memorie în ordine **crescătoare a dimensiunii** și **a adreselor**.

---

## 🛠️ **Descrierea implementării**
Pentru gestionarea eficientă a memoriei, programul folosește mai multe **structuri de date**:

### 🔹 **Structuri de date utilizate**
- **`doubly_linked_list_t`** → Reprezintă o **listă dublu înlănțuită** pentru blocurile de memorie.
- **`dll_node_t`** → Un **nod** din lista dublu înlănțuită, reținând blocurile de memorie.
- **`block_t`** → Un **bloc de memorie**, conținând:
  - **Adresa** blocului
  - **Dimensiunea** blocului
  - **Datele stocate** (opțional)
- **`sfl_t`** → Structura **Segregated Free Lists**, conținând:
  - Listele dublu înlănțuite pentru **blocurile libere**.
  - O **listă separată** pentru blocurile **alocate**.

---

### 🔹 **Detalii implementare**
#### 📌 **Inițializarea heap-ului (`INIT_HEAP`)**
- Se creează **vectorul de liste dublu înlănțuite** pentru blocurile de memorie libere.
- Se asociază fiecare listă unei **dimensiuni de bloc** (puteri ale lui 2).
- Blocurile inițiale sunt **adăugate** în listele corespunzătoare.

#### 🏗️ **Alocarea memoriei (`MALLOC`)**
- Se caută un bloc liber de **dimensiune minimă suficientă**.
- Dacă blocul este **mai mare**, se **fragmentează**, iar partea neutilizată revine în lista corespunzătoare.
- Dacă **nu există memorie disponibilă**, se afișează **"Out of memory"**.

#### ♻️ **Eliberarea memoriei (`FREE`)**
- Blocul este scos din **lista blocurilor alocate** și introdus în **Segregated Free Lists**.
- Dacă **tip_reconstituire** este `1`, se încearcă **reconstituirea** blocurilor fragmentate.
- Dacă adresa **nu este validă**, se afișează **"Invalid free"**.

#### 🔢 **Citire și scriere (`READ` / `WRITE`)**
- Se verifică **dacă zona a fost alocată**.
- Dacă nu, se afișează **"Segmentation fault (core dumped)"**.
- Scrierea și citirea se fac doar în **blocuri alocate**.

#### 🛠️ **Dump-ul memoriei (`DUMP_MEMORY`)**
- Se afișează:
  - **Memorie totală, liberă și alocată**.
  - **Numărul de blocuri libere și alocate**.
  - **Apelurile `MALLOC`, `FREE` și fragmentările**.
- Se listează blocurile **în ordine crescătoare**.

---

## 🏁 **Exemplu de utilizare**
```bash
> INIT_HEAP 0x1000 4 128 1
> MALLOC 16
Allocated at 0x1000
> MALLOC 32
Allocated at 0x1010
> WRITE 0x1000 "Hello World!" 12
> READ 0x1000 12
Hello World!
> FREE 0x1000
Block at 0x1000 freed
> DUMP_MEMORY
+++++DUMP+++++
Total memory: 512 bytes
Total allocated memory: 32 bytes
Total free memory: 480 bytes
Number of free blocks: 3
Number of allocated blocks: 1
Number of malloc calls: 2
Number of fragmentations: 1
Number of free calls: 1
Allocated blocks: (0x1010 - 32)
-----DUMP-----
> DESTROY_HEAP
Heap destroyed
```

---

## 🎯 **Concluzie**
Acest proiect implementează un allocator de memorie eficient, bazat pe Segregated Free Lists, gestionând blocuri de memorie prin liste dublu înlănțuite. Structura modularizată asigură performanță optimă, iar utilizarea fragmentării și reconstituirii oferă o mai bună utilizare a memoriei.

🚀 **Proiect realizat cu pasiune pentru Structuri de Date si Algoritmi!** 🚀
