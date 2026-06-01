// File WL_Helpers.cc
#include "WL_Helpers.hh"

WL_SolutionManager::WL_SolutionManager(const WL_Input & pin) 
  : SolutionManager<WL_Input,WL_Output>(pin, "WLSolutionManager")  {} 

void WL_SolutionManager::RandomState(WL_Output& out) //genera una soluzione iniziale casuale, assegnando ogni negozio ad un magazzino scelto casualmente
{
  unsigned s, w; //dichiara due variabili intere senza segno s negozio w magazzino
  out.Reset(); //scollega tutti i negozi dai magazzini, azzera i carichi dei magazzini e il costo totale
  for (s = 0; s < in.Stores(); s++) //per ogni negozio s
    {
      w = Random::Uniform<int>(0, in.Warehouses() - 1); //estrae un numero intero casuale tra 0 e il numero di magazzini -1
      out.Assign(s,w); //assegna il negozio s al magazzino w
    }
} 

void WL_SolutionManager::GreedyState(WL_Output& out) //genera una soluzione iniziale assegnando ogni negozio al magazzino che lo serve al costo più basso, tenendo conto del costo di apertura del magazzino e del costo di fornitura
{
  bool found_first; //variabile booleana che indica se è stata trovata una prima assegnazione fattibile per un negozio, inizialmente falsa
  unsigned i, s, w, best_s, best_i, best_w;
  double cost, best_cost, amortized_fixed_cost;
  vector<unsigned> unserved_stores(in.Stores()); //lista di negozi non ancora serviti

  out.Reset(); //scollega tutti i negozi dai magazzini, azzera i carichi dei magazzini e il costo totale
  iota(unserved_stores.begin(), unserved_stores.end(),0); //riempie la lista dei negozi non ancora serviti con i numeri da 0 a numero di negozi -1 (LA LISTA QUINDI INIZIALMENTE CONTIENE TUTTI I NEGOZI)

  while(unserved_stores.size() > 0) //finché ci sono negozi non ancora serviti
    {
      found_first = false;
      for (i = 0; i < unserved_stores.size(); i++) //scorre la lista dei negozi non ancora serviti
        {
          s = unserved_stores[i]; //prende il negozio s dalla lista dei negozi non ancora serviti
          for (w = 0; w < in.Warehouses(); w++) //scorre tutti i magazzini
            if (out.ResidualCapacity(w) > in.AmountOfGoods(s)) //se il magazzino w ha capacità residua maggiore della quantità di beni richiesta dal negozio s
              {
                if (out.Load(w) == 0) //se il magazzino w è chiuso, calcola il costo fisso ammortizzato per servire il negozio s dal magazzino w
                  amortized_fixed_cost = in.FixedCost(w) * in.AmountOfGoods(s)/static_cast<double>(in.Capacity(w)); //il costo fisso ammortizzato è il costo fisso del magazzino w moltiplicato per la quantità di beni richiesta dal negozio s diviso per la capacità totale del magazzino w, in modo da distribuire il costo fisso proporzionalmente alla quantità di beni servita
                else
                  amortized_fixed_cost = 0.0;
                if (!found_first) //se non è stata trovata una prima assegnazione fattibile per il negozio s, allora questa è la migliore finora, quindi la memorizza come best_w, best_s, best_i e best_cost
                  {
                    found_first = true;
                    best_w = w;
                    best_s = s;
                    best_i = i;
                    best_cost = in.SupplyCost(s,w) + amortized_fixed_cost;
                  }
                else
                  {
                    cost = in.SupplyCost(s,w) + amortized_fixed_cost;
                    if (cost < best_cost) //se il costo di servire il negozio s dal magazzino w è inferiore al costo della migliore assegnazione trovata finora, allora aggiorna best_w, best_s, best_i e best_cost con i nuovi valori
                      {
                        best_w = w;
                        best_s = s;
                        best_i = i;
                        best_cost = cost;
                      }
                  }
              }
        }
      if (!found_first) //se non è stata trovata nessuna assegnazione fattibile per il negozio s (vuol dire che tutti i magazzini sono sovraccarichi), allora esce dal ciclo while, lasciando i negozi rimanenti non assegnati
        break; // no feasible assignment found
      out.Assign(best_s,best_w); //assegna il negozio best_s al magazzino best_w, aggiornando i carichi dei magazzini e il costo totale
      unserved_stores.erase(unserved_stores.begin() + best_i); //rimuove il negozio best_s dalla lista dei negozi non ancora serviti, usando l'indice best_i che è stato memorizzato quando è stata trovata la migliore assegnazione per quel negozio
    }
  if (unserved_stores.size() > 0) // se ci sono ancora negozi non assegnati (vuol dire che tutti i magazzini sono sovraccarichi), allora li assegna al magazzino 0 (che ovviamente vedrà sovraccaricarsi, ma almeno la soluzione sarà completa e non ci saranno negozi non serviti da dare al local search. Ovviamente ci sarà un costo molto alto, ma è solo una soluzione iniziale, che poi sarà migliorata dal local search)
    for (i = 0; i < unserved_stores.size(); i++)
      out.Assign(unserved_stores[i], 0);
}

/*Ecco l'esatta sequenza degli eventi che rende necessario quell'if finale:

Il ciclo normale: L'algoritmo sta girando dentro il grande while. Trova posto per il negozio A, poi per il negozio B, poi per il negozio C. Ogni volta la lista unserved_stores si accorcia.

Il blocco: A un certo punto, restano 3 negozi da sistemare, ma i magazzini sono tutti pieni fino all'orlo. L'algoritmo guarda ovunque ma found_first rimane false.

L'evacuazione (break): Se non ci fosse il break, l'algoritmo cercherebbe di fare un'assegnazione a vuoto o andrebbe in un ciclo infinito. Il break interviene e dice: "Ok, non c'è più spazio legale. Smettila di provare. Esci dal ciclo while".

L'atterraggio: Uscendo dal ciclo while, il programma "atterra" esattamente sulla riga successiva, che è proprio il nostro if (unserved_stores.size() > 0).

A questo punto, ci sono due scenari possibili per cui siamo arrivati lì, e l'if serve a distinguerli:

Scenario A (Tutto bene): Il while ha finito il suo lavoro naturalmente perché ha azzerato la lista. In questo caso unserved_stores.size() è 0. L'if è falso, il programma lo salta e la funzione termina felicemente.

Scenario B (Siamo scappati col break): Siamo usciti in anticipo dal ciclo proprio a causa del break. La lista NON è vuota (ci sono quei famosi 3 negozi sfortunati). Qui l'if è vero (> 0) e scatta la trappola: l'algoritmo prende i superstiti e li sbatte nel magazzino 0 senza fare complimenti.*/

bool WL_SolutionManager::CheckConsistency(const WL_Output& st) const
{ // not necessary, as state modifications are managed by the single method Assign
  return true;
}

//Nel framework EasyLocal++, il metodo CheckConsistency (che significa "Controlla la Coerenza") è stato progettato come un sistema di allarme.

//In problemi molto complessi, una soluzione potrebbe essere composta da tantissime liste e tabelle intrecciate. Potrebbe capitare un errore logico (un bug) in cui il programma dice: "Il negozio 5 è nel Magazzino 1", ma poi si dimentica di aggiungere il peso del negozio 5 al totale del Magazzino 1. In quel caso, la soluzione è "incoerente" (i conti non tornano). CheckConsistency servirebbe a scovare questi errori.
//Questo commento lasciato dal professore spiega il trucco: "Non è necessario, perché le modifiche allo stato sono gestite dall'unico metodo Assign".
//Se andassimo a guardare il file WL_Data.cc, vedremmo che l'unico modo permesso a tutto il programma per spostare un negozio è usare il comando out.Assign(...). Quel comando è stato programmato in modo blindato e infallibile: quando sposta un negozio, aggiorna istantaneamente e contemporaneamente sia la lista delle assegnazioni sia i pesi dei magazzini.

int WL_Supply::ComputeCost(const WL_Output& st) const //calcola il costo totale di fornitura, riceve in ingresso st che sarebbe status che indica la soluzione corrente, chi è stato assegnato a chi 
{
  unsigned s, cost = 0;
  for (s = 0; s < in.Stores(); s++) //passa in rassegna tutti i negozi s
    cost += in.AmountOfGoods(s) * in.SupplyCost(s,st.Assignment(s)); //per ogni negozio s, calcola il costo di fornitura per quel negozio, che è dato dalla quantità di beni richiesta dal negozio s moltiplicata per il costo di fornitura per quel negozio s dal magazzino a cui è assegnato (st.Assignment(s) restituisce il magazzino a cui è assegnato il negozio s)
  return cost;                                                       //il costo totale di ogni negozio viene sommato al costo totale complessivo, che alla fine viene restituito come risultato della funzione.
}

void WL_Supply::PrintViolations(const WL_Output& st, ostream& os) const //stampa i dettagli dei costi di fornitura per ogni negozio, indicando la quantità di beni richiesta, il magazzino a cui è assegnato, il costo totale di fornitura e il costo per unità
//è void perché non restituisce nulla, ma stampa direttamente su schermo
{
  unsigned s;
  for (s = 0; s < in.Stores(); s++) //reminder: AmountOfGoods restituisce la quantità di beni richiesta per quel negozio
    os << "The cost of supplying " << in.AmountOfGoods(s) << " units from " << st.Assignment(s) //i simboli << serve per unire pezzi di testo fisso con i numeri presi dai dati in modo da avere una frase completa  
       << " to " << s << " is " << in.AmountOfGoods(s)*in.SupplyCost(s,st.Assignment(s)) 
       << " (cost per unit " << in.SupplyCost(s,st.Assignment(s)) << ")" << endl;	
}

int WL_Opening::ComputeCost(const WL_Output& st) const
{ 
  unsigned w, cost = 0;
  for (w = 0; w < in.Warehouses(); w++) //passa in rassegna tutti i magazzini w
    if (st.Load(w) > 0) //se il magazzino w ha un carico maggiore di 0 (vuol dire che è aperto, perché se fosse chiuso non potrebbe avere carico), allora aggiunge al costo totale il costo fisso di apertura del magazzino w (in.FixedCost(w) restituisce il costo fisso di apertura del magazzino w)
      cost += in.FixedCost(w);
  return cost;
}

void WL_Opening::PrintViolations(const WL_Output& st, ostream& os) const
{
  unsigned w;
  for (w = 0; w < in.Warehouses(); w++)
    if (st.Load(w) > 0)
      os << "The cost of opening warehouse " << w << " is " << in.FixedCost(w) << endl;
}

int WL_Capacity::ComputeCost(const WL_Output& st) const //calcola la multa, cioè il costo totale dovuto al sovraccarico dei magazzini, riceve in ingresso st che sarebbe status che indica la soluzione corrente, chi è stato assegnato a chi
{ 
  unsigned w, cost = 0;
  for (w = 0; w < in.Warehouses(); w++)
    if (st.Load(w) > in.Capacity(w))
      cost += st.Load(w) - in.Capacity(w); //se il carico del magazzino w è maggiore della sua capacità, allora aggiunge al costo totale la quantità di sovraccarico, che è data dal carico del magazzino w meno la capacità del magazzino w. In questo modo, più un magazzino è sovraccarico, più costa, e se un magazzino non è sovraccarico (carico <= capacità) non aggiunge nulla al costo totale.
  return cost;
}
 
void WL_Capacity::PrintViolations(const WL_Output& st, ostream& os) const
{
  unsigned w;
  for (w = 0; w < in.Warehouses(); w++)
    if (st.Load(w) > in.Capacity(w))
      os << "Warehouse " << w << " is overloaded by " << st.Load(w) - in.Capacity(w) << endl;
}

/*****************************************************************************
 * WL_Change Neighborhood Methods
 *****************************************************************************/
WL_Change::WL_Change() //metodo chiamato quando c'è da fare una mossa, cioè quando c'è da spostare un negozio da un magazzino a un altro. Inizializza le variabili store, old_w e new_w a -1, che è un valore di default che indica che non è stata ancora specificata la mossa concreta (non è stato ancora detto quale negozio spostare e da quale magazzino a quale magazzino). 
//Quindi, quando viene creato un oggetto WL_Change, le sue variabili sono tutte impostate a -1, e poi quando si decide di fare una mossa concreta, si assegnano i valori corretti a queste variabili.
{
   store = -1;
   old_w = -1;
   new_w = -1;
}

bool operator==(const WL_Change& mv1, const WL_Change& mv2) //metodo usato per capire se due mosse (mv1 e mv2)sono uguali, cioè se spostano lo stesso negozio dallo stesso magazzino di origine allo stesso magazzino di destinazione. 
//Restituisce true se le due mosse sono uguali, altrimenti restituisce false
//due mosse sono uguali se spostano lo stesso negozio (mv1.store == mv2.store) allo stesso magazzino di destinazione (mv1.new_w == mv2.new_w)
{
  return mv1.store == mv2.store && mv1.new_w == mv2.new_w;
}

bool operator!=(const WL_Change& mv1, const WL_Change& mv2) //metodo usato per capire se due mosse (mv1 e mv2)sono diverse
//cioè se spostano negozi diversi o se spostano lo stesso negozio verso magazzini di destinazione diversi
{
  return mv1.store != mv2.store || mv1.new_w != mv2.new_w;
}

bool operator<(const WL_Change& mv1, const WL_Change& mv2) //metodo usato per ordinare le mosse (risponde alla domanda: la mossa 1 deve essere messa in lista prima della mossa 2?)
{
  return (mv1.store < mv2.store) //se la mossa 1 sposta un negozio con indice più basso rispetto alla mossa 2, allora la mossa 1 viene considerata "minore" della mossa 2 e quindi viene messa prima in lista
  || (mv1.store == mv2.store && mv1.new_w < mv2.new_w); //se le due mosse spostano lo stesso negozio (mv1.store == mv2.store), allora si confrontano i magazzini di destinazione: se la mossa 1 sposta il negozio verso un magazzino con indice più basso rispetto alla mossa 2, allora la mossa 1 viene considerata "minore" della mossa 2 e quindi viene messa prima in lista
}

istream& operator>>(istream& is, WL_Change& mv) //metodo usato per leggere una mossa da un input stream (ad esempio, da un file o da tastiera) e memorizzarla in un oggetto WL_Change (mv).
{
  char ch;
  is >> mv.store >> ch >> mv.old_w >> ch >> ch >> mv.new_w; //si aspetta che l'input sia nel formato "store:old_w->new_w", ad esempio "5:2->3". Quindi, legge prima il numero del negozio (mv.store), poi legge un carattere (ch) che dovrebbe essere ':', poi legge il numero del magazzino di origine (mv.old_w), poi legge un altro carattere (ch) che dovrebbe essere '-', poi legge un altro carattere (ch) che dovrebbe essere '>', e infine legge il numero del magazzino di destinazione (mv.new_w). 
  //Se l'input è correttamente formattato, allora mv conterrà i valori corretti per store, old_w e new_w. Se l'input non è correttamente formattato, allora mv potrebbe contenere valori errati o potrebbe causare un errore di input.
  return is;
}

ostream& operator<<(ostream& os, const WL_Change& mv) //metodo usato per stampare una mossa in modo leggibile
{
  os << mv.store << ':' << mv.old_w << "->" << mv.new_w; //esempio di output: "5:2->3" significa che la mossa sposta il negozio 5 dal magazzino 2 al magazzino 3
  return os;
}

void WL_ChangeNeighborhoodExplorer::RandomMove(const WL_Output& st, WL_Change& mv) const //genera una mossa casuale
{
  // Sceglie un negozio a caso
  //i :: indicano a che famiglia deve appartenere il comando (tipo uniform deve appartenere alla famiglia Random)
  //uniform indica che vogliamo una distribuzione uniforme, cioè che ogni negozio ha la stessa probabilità di essere scelto, e int indica che vogliamo un numero intero
  mv.store = Random::Uniform<int>(0, in.Stores() - 1); //chiede al generatore di numeri casuali di restituire un numero intero compreso tra 0 e il numero di negozi meno 1, e assegna questo numero dentro la variabile store della nostra mossa
  mv.old_w = st.Assignment(mv.store); // Prende il magazzino attuale a cui è assegnato quel negozio
  
  // Sceglie un nuovo magazzino a caso, assicurandosi che sia diverso da quello attuale
  do {
    mv.new_w = Random::Uniform<int>(0, in.Warehouses() - 1); 
  } while (mv.new_w == mv.old_w); // Continua a generare un nuovo magazzino finché non è diverso da quello attuale
} 

bool WL_ChangeNeighborhoodExplorer::FeasibleMove(const WL_Output& st, const WL_Change& mv) const //controlla se la mossa è fattibile, riceve in input la situazione attuale st e la mossa
{
  // Una mossa ha senso solo se sposto il negozio in un magazzino diverso
  //ATTENZIONE: il metodo precedente valutava se due mosse erano uguali o diverse, questo valuta la singola mossa 
  return mv.new_w != mv.old_w;
} 

void WL_ChangeNeighborhoodExplorer::MakeMove(WL_Output& st, const WL_Change& mv) const //applica la mossa alla soluzione corrente, riceve in input la situazione attuale st e la mossa da applicare mv
{
  // Esegue la mossa usando il metodo Assign fornito dalla classe WL_Output
  st.Assign(mv.store, mv.new_w);
}  

/*1) Chiama RandomMove per inventarsi uno spostamento.

2) Chiama FeasibleMove per assicurarsi che non sia una finta mossa (es. 1 -> 1).

3) (Passaggio segreto: calcola se la mossa fa risparmiare soldi). manca questa (delta cost).

4)Se la mossa è vantaggiosa, chiama MakeMove per renderla definitiva!*/

void WL_ChangeNeighborhoodExplorer::FirstMove(const WL_Output& st, WL_Change& mv) const //genera la prima mossa da esplorare, riceve in input la situazione attuale st e la mossa da generare mv
{
  // La prima mossa in assoluto: prendo il negozio 0 e provo ad assegnarlo al magazzino 0
  mv.store = 0;
  mv.old_w = st.Assignment(mv.store);
  mv.new_w = 0;
  
  // Se era già nel magazzino 0, provo col magazzino 1
  if (mv.new_w == mv.old_w) {
    mv.new_w = 1;
  }
}

bool WL_ChangeNeighborhoodExplorer::NextMove(const WL_Output& st, WL_Change& mv) const //genera la mossa successiva da esplorare, riceve in input la situazione attuale st e la mossa da generare mv
{
  do
    if (!AnyNextMove(st,mv)) //anynextmove è un "contachilometri", passa da negozio 0 magazzino 1 a negozio 0 magazzino 2, fino a negozio 0 magazzino W-1, poi passa a negozio 1 magazzino 0, e così via. 
    //Se AnyNextMove restituisce false, significa che abbiamo finito di esplorare tutte le mosse possibili
      return false;
  while (!FeasibleMove(st,mv)); //continue a generare nuove mosse finché non ne trova una fattibile (es. 1 -> 1 non è fattibile, ma 1 -> 2 lo è)
  return true; //se siamo arrivati qua significa che anynextmove ha trovato una sol fattibile e che è feasiblemove, quindi restituisce true per dire che c'è una nuova mossa da esplorares
}

bool WL_ChangeNeighborhoodExplorer::AnyNextMove(const WL_Output& st, WL_Change& mv) const //"contachilometri" che genera la prossima mossa in ordine, riceve in input la situazione attuale st e la mossa da generare mv
{
  // Iteratore: provo a incrementare il magazzino di destinazione
  mv.new_w++;
  
  // Se ho finito i magazzini, passo al negozio successivo e riparto dal magazzino 0
  if (mv.new_w >= (int)in.Warehouses()) { //(int) è un cast, serve per convertire il numero di magazzini (che è di tipo unsigned) in un intero, così posso confrontarlo con mv.new_w che è di tipo int (perchè myv.new_w può essere -1, quindi interi positivi o negativi, mentre in.Warehouses() è sempre positivo essendo unsigned)
    mv.store++;
    
    // Se ho finito anche i negozi, significa che ho esplorato tutto l'intorno (vicinato) e restituisco false
    if (mv.store >= (int)in.Stores()) //mv.store è di tipo int, in.Stores() è di tipo unsigned, quindi serve il cast per confrontarli anche se tecnicamente a questo punto mv.store non dovrebbe mai essere negativo, ma se non mettiamo il compilatore da warning)
    {   
      return false;
    }
    
    mv.old_w = st.Assignment(mv.store); 
    mv.new_w = 0;
  }
  return true;
}

//invece di ricalcolare da zero il costo totale della soluzione dopo aver applicato una mossa, si calcola la differenza di costo (delta) che quella mossa comporta, e poi sommare questa differenza al costo attuale per ottenere il nuovo costo.

int WL_ChangeDeltaSupply::ComputeDeltaCost(const WL_Output& st, const WL_Change& mv) const
{
  // Differenza di costo per il Rifornimento (Soft)
  int goods = in.AmountOfGoods(mv.store); //quantità di merce richiesta dal negozio che stiamo spostando
  int old_cost = goods * in.SupplyCost(mv.store, mv.old_w); //quanto stiamo pagando attualmente per rifornire quel negozio dal magazzino di origine (old_w)
  int new_cost = goods * in.SupplyCost(mv.store, mv.new_w); //quanto pagheremmo se rifornissimo quel negozio dal magazzino di destinazione (new_w)
  
  return new_cost - old_cost; 
}

//il seguente metodo risponde a due domande: 1) questa mossa mi fa svuotare del tutto un magazzino che prima usavo? 2) questa mossa mi fa usare un magazzino che prima era vuoto?

int WL_ChangeDeltaOpening::ComputeDeltaCost(const WL_Output& st, const WL_Change& mv) const
{
  // Differenza di costo per l'Apertura/Chiusura (Soft)
  int delta = 0;
  int goods = in.AmountOfGoods(mv.store);
  
  // Guardo il vecchio magazzino da cui il negozio sta "scappando" 
  if (st.Load(mv.old_w) == goods && goods > 0) //se st.load (carico del magazzino di origine) è uguale alla quantità di beni richiesta dal negozio che stiamo spostando, allora vuol dire che dopo aver spostato quel negozio, il magazzino di origine si svuota completamente, quindi dobbiamo togliere il costo fisso di quel magazzino (perché se un magazzino è vuoto, non lo paghiamo)
  {
    delta -= in.FixedCost(mv.old_w);
  }
  
  // Se il nuovo magazzino era vuoto e ci metto questo negozio, devo pagare il suo costo fisso
  if (st.Load(mv.new_w) == 0 && goods > 0) 
  {
    delta += in.FixedCost(mv.new_w);
  }
  
  return delta;
}

int WL_ChangeDeltaCapacity::ComputeDeltaCost(const WL_Output& st, const WL_Change& mv) const //metodo dedicato al costo di superamento della capacità
{
  // Differenza di costo per il Superamento Capacità (Hard)
  int goods = in.AmountOfGoods(mv.store);
  int old_w_load = st.Load(mv.old_w);
  int new_w_load = st.Load(mv.new_w);
  int old_w_cap = in.Capacity(mv.old_w);
  int new_w_cap = in.Capacity(mv.new_w);

  // Calcolo quanto ammonta la multa attuale, prima di fare la mossa
  //max(0,...) 
  //se il magazzino ha 120 pacchi, ma la capacità è 100, fa 120 - 100 = 20 quindi max (0,20) = 20, cioè c'è una multa di 20. 
  //Se invece il magazzino ha 80 pacchi e la capacità è 100, fa 80 - 100 = -20 quindi max(0,-20) = 0, cioè non c'è multa perché non è sovraccarico. 
  //In questo modo, se un magazzino non è sovraccarico, non contribuisce al costo totale, mentre se è sovraccarico, contribuisce con la quantità di sovraccarico.
  //max(0, new_w_load + goods - new_w_cap) serve per calcolare la multa attuale del nuovo magazzino che ha già prima di effettuare la mossa, e che potrebbe essere sovraccarico anche prima di spostare il negozio, e se è sovraccarico, contribuisce al costo totale con la quantità di sovraccarico.
  int cost_before = max(0, old_w_load - old_w_cap) + max(0, new_w_load - new_w_cap);
  
  // Calcolo la penalità futura DOPO lo spostamento
  int cost_after = max(0, old_w_load - goods - old_w_cap) + max(0, new_w_load + goods - new_w_cap);

  return cost_after - cost_before;
}

