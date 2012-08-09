// Start test4

#include "server.h"

Server::Server()
{
        head = NULL;
}

int Server::port_control(int port)
{ 
        if(port <= 1023) {
                cerr << "Porta <" << port << "> non utilizzabile :: Well Known Port" << endl;
                return -1;
        }

        if(port <= 65535 && port >= 49152) {
                cout << "Fare attenzione nella scelta della porta <" << port << "> :: Dynamic and/or Private Port" << endl;
        }

        return 1;
}

void Server::read_config_file(const char *config_file)
{
        char *tmp;
        int tmp_port;
        int tmp_block;
        fstream fc;

        fc.open(config_file, ios::in);
        if(!fc) {
                cerr << "Errore nell'apertura del file" << endl;
                return;
        }

        while (fc.getline(line, MAX_LINE_LEN)) { /* Legge riga per riga il contenuto del file di configurazione */
                tmp = strtok(line, ":");
                if(tmp == NULL) /* Se incontra una riga vuota passa al ciclo successivo */
                        continue;
                if(strcmp(tmp, SERVER_IP_ADD) == 0) {
                        tmp = strtok(NULL, ":");
                        if(tmp == NULL)
                                continue;
                        tmp_port = atoi(tmp);
                        if(tmp_port == SERVER_PORT)
                                while((tmp = strtok(NULL, ":"))) {
                                        if(tmp == NULL)
                                                continue;
                                        tmp_block = atoi(tmp);
                                        add_block(tmp_block);
                                }
                }               
        }

        fc.close();
}

void Server::add_block(int id_block)
{
        block* elem;
        block* tmp_ptr;

        for(elem = head; elem != NULL; elem = elem->succ)
                tmp_ptr = elem;

        elem = new block;       
        elem->ID = id_block;
        elem->pointer = malloc(DIMBLOCK);
        bzero(elem->pointer, sizeof(elem->pointer));
        pthread_mutex_init(&elem->b_acc, NULL);
        elem->succ = NULL;
        elem->cl_ptr = NULL;

        if(head == NULL)
                head = elem;
        else
                tmp_ptr->succ = elem;   
}

block * Server::find_block(int id)
{
        block *bk_elem;

        for(bk_elem = head; bk_elem->ID != id; bk_elem = bk_elem->succ)
                if(!bk_elem->succ)
                        return bk_elem->succ;
        return bk_elem;
}

client_list * Server::find_client(int client_id, block *bk_elem)
{
        client_list *cl_elem;

        for(cl_elem = bk_elem->cl_ptr; cl_elem->cl_ID != client_id; cl_elem = cl_elem->succ)
                if(!cl_elem->succ)
                        return cl_elem->succ;
        return cl_elem;
}

void Server::add_client(int client_id, block *bk_elem)
{
        client_list *cl_elem = new client_list;
        cl_elem->cl_ID = client_id;
        cl_elem->valid = 1;
        cl_elem->lock = 0;
        cl_elem->succ = bk_elem->cl_ptr;
        bk_elem->cl_ptr = cl_elem;
}

void Server::delete_client(int client_id, block *bk_elem)
{
        client_list *cl_elem;
        client_list *cl_elem_aux;

        for(cl_elem = bk_elem->cl_ptr; cl_elem->cl_ID != client_id; cl_elem = cl_elem->succ)
                cl_elem_aux = cl_elem;

        if(bk_elem->cl_ptr == cl_elem)
                bk_elem->cl_ptr = cl_elem->succ;
        else
                cl_elem_aux->succ = cl_elem->succ;
        
        delete cl_elem;
}

void Server::change_valid(int client_id, block *bk_elem)
{
        client_list *cl_elem;

        for(cl_elem = bk_elem->cl_ptr; cl_elem != NULL; cl_elem = cl_elem->succ)
                if(cl_elem->cl_ID != client_id)
                        cl_elem->valid = 0;
}

int Server::client_unblock(int client_id, block *bk_elem)
{
        int len;
        int ret;
        client_list *cl_elem;

        for(cl_elem = bk_elem->cl_ptr; cl_elem != NULL; cl_elem = cl_elem->succ)
                if(cl_elem->cl_ID != client_id)
                        if(cl_elem->lock) {
                                len = sizeof(cl_elem->lock);
                                ret = send(cl_elem->cl_ID, &cl_elem->lock, len, 0);
                                if(ret == -1) {
                                        cerr << "Errore: invio sblocco wait fallito nella <client_unblock>" << endl;    
                                        return -1;
                                }
                                cl_elem->lock = 0;
                        }
        return 1;
}

int Server::block_map(int sk)
{
        int ret;
        int len;
        int id;

        len = sizeof(id);
        ret = recv(sk, &id, len, MSG_WAITALL);
        if((ret == -1) || (ret < len)) {
                cerr << "Errore: ricezione ID fallita nella <block_map>" << endl;       
                return -1;
        }       

        block *bk_elem = find_block(id);
        if(bk_elem == NULL) {
                cerr << "Errore: blocco non trovato nella <block_map>" << endl; 
                return -1;
        }
                
        pthread_mutex_lock(&bk_elem->b_acc);

        add_client(sk, bk_elem);

        ret = send(sk, bk_elem->pointer, DIMBLOCK, 0);
        if(ret == -1) {
                cerr << "Errore: invio blocco di memoria fallito nella <block_map>" << endl;
                pthread_mutex_unlock(&bk_elem->b_acc);
                return -1;
        }

        pthread_mutex_unlock(&bk_elem->b_acc);

        return 1;
}

int Server::block_unmap(int sk)
{
        int ret;
        int len;
        int id;

        len = sizeof(id);
        ret = recv(sk, &id, len, MSG_WAITALL);
        if((ret == -1) || (ret < len)) {
                cerr << "Errore: ricezione ID fallita nella <block_unmap>" << endl;     
                return -1;
        }
        
        block *bk_elem = find_block(id);
        if(bk_elem == NULL) {
                cerr << "Errore: blocco non trovato nella <block_unmap>" << endl;       
                return -1;
        }

        pthread_mutex_lock(&bk_elem->b_acc);

        delete_client(sk, bk_elem);

        pthread_mutex_unlock(&bk_elem->b_acc);

        return 1;
}

int Server::block_update(int sk)
{
        int ret;
        int len;
        int id;

        len = sizeof(id);
        ret = recv(sk, &id, len, MSG_WAITALL);
        if((ret == -1) || (ret < len)) {
                cerr << "Errore: ricezione ID fallita nella <block_update>" << endl;            
                return -1;
        }

        block *bk_elem = find_block(id);
        if(bk_elem == NULL) {
                cerr << "Errore: blocco non trovato nella <block_update>" << endl;      
                return -1;
        }

        pthread_mutex_lock(&bk_elem->b_acc);

        client_list *cl_elem = find_client(sk, bk_elem);
        if(cl_elem == NULL) {
                cerr << "Errore: client non trovato nella <block_update>" << endl;      
                pthread_mutex_unlock(&bk_elem->b_acc);
                return -1;
        }       

        if(cl_elem->valid) {
                len = sizeof(cl_elem->valid);
                ret = send(sk, &cl_elem->valid, len, 0);
                if(ret == -1) {
                        cerr << "Errore: invio controllo validita' fallito nella <block_update>" << endl;       
                        pthread_mutex_unlock(&bk_elem->b_acc);
                        return -1;
                }       

                pthread_mutex_unlock(&bk_elem->b_acc);
                
                return 1;
        }
        else {
                len = sizeof(cl_elem->valid);
                ret = send(sk, &cl_elem->valid, len, 0);
                if(ret == -1) {
                        cerr << "Errore: invio controllo validita' fallito nella <block_update>" << endl;       
                        pthread_mutex_unlock(&bk_elem->b_acc);
                        return -1;
                }
        }

        ret = send(sk, bk_elem->pointer, DIMBLOCK, 0);
        if(ret == -1) {
                cerr << "Errore: invio blocco di memoria fallito nella <block_update>" << endl; 
                pthread_mutex_unlock(&bk_elem->b_acc);
                return -1;
        }
        
        cl_elem->valid = 1;

        pthread_mutex_unlock(&bk_elem->b_acc);

        return 1;
}

int Server::block_write(int sk)
{
        int ret;
        int len;
        int id; 

        len = sizeof(id);
        ret = recv(sk, &id, len, MSG_WAITALL);
        if((ret == -1) || (ret < len)) {
                cerr << "Errore: ricezione ID fallita nella <block_write>" << endl;             
                return -1;
        }               
        
        block *bk_elem = find_block(id);
        if(bk_elem == NULL) {
                cerr << "Errore: blocco non trovato nella <block_write>" << endl;       
                return -1;
        }

        pthread_mutex_lock(&bk_elem->b_acc);    

        client_list *cl_elem = find_client(sk, bk_elem);
        if(cl_elem == NULL) {
                cerr << "Errore: client non trovato nella <block_write>" << endl;       
                pthread_mutex_unlock(&bk_elem->b_acc);
                return -1;
        }

        if(cl_elem->valid) {
                len = sizeof(cl_elem->valid);
                ret = send(sk, &cl_elem->valid, len, 0);
                if(ret == -1) {
                        cerr << "Errore: invio controllo validita' fallito nella <block_write>" << endl;        
                        pthread_mutex_unlock(&bk_elem->b_acc);
                        return -1;
                }       
        }
        else {
                len = sizeof(cl_elem->valid);
                ret = send(sk, &cl_elem->valid, len, 0);
                if(ret == -1) {
                        cerr << "Errore: invio controllo validita' fallito nella <block_write>" << endl;        
                        pthread_mutex_unlock(&bk_elem->b_acc);
                        return -1;
                }

                pthread_mutex_unlock(&bk_elem->b_acc);          

                return 1;               
        }

        len = DIMBLOCK;
        ret = recv(sk, bk_elem->pointer, len, MSG_WAITALL);
        if((ret == -1) || (ret < len)) {
                cerr << "Errore: ricezione blocco di memoria fallita nella <block_write>" << endl;      
                pthread_mutex_unlock(&bk_elem->b_acc);  
                return -1;
        }

        change_valid(sk, bk_elem);

        ret = client_unblock(sk, bk_elem);
        if(ret == -1) {
                pthread_mutex_unlock(&bk_elem->b_acc);
                return -1;
        }

        pthread_mutex_unlock(&bk_elem->b_acc);
        
        return 1;
}

int Server::block_wait(int sk)
{
        int ret;
        int len;
        int id;

        len = sizeof(id);
        ret = recv(sk, &id, len, MSG_WAITALL);
        if((ret == -1) || (ret < len)) {
                cerr << "Errore: ricezione ID fallita nella <block_wait>" << endl;              
                return -1;
        }

        block *bk_elem = find_block(id);
        if(bk_elem == NULL) {
                cerr << "Errore: blocco non trovato nella <block_wait>" << endl;        
                return -1;
        }
        
        pthread_mutex_lock(&bk_elem->b_acc);

        client_list *cl_elem = find_client(sk, bk_elem);
        if(cl_elem == NULL) {
                cerr << "Errore: client non trovato nella <block_wait>" << endl;        
                pthread_mutex_unlock(&bk_elem->b_acc);
                return -1;
        }

        len = sizeof(cl_elem->valid);
        ret = send(sk, &cl_elem->valid, len, 0);
        if(ret == -1) {
                cerr << "Errore: invio controllo validita' fallito nella <block_wait>" << endl; 
                pthread_mutex_unlock(&bk_elem->b_acc);
                return -1;
        }

        if(cl_elem->valid)
                cl_elem->lock = 1;

        pthread_mutex_unlock(&bk_elem->b_acc);

        return 1;
}

struct argument {
        Server s; /* Oggetto classe Server */
        int use_sk; /* Socket associato al thread */
};

void* thread_body(void *arg)
{
        int ret;
        int len;
        char command;

        argument tmp_arg = *(argument *)arg;            
        int tmp_sk = tmp_arg.use_sk;
        Server serv = tmp_arg.s;

        pthread_mutex_unlock(&m_acc);

        while(1) {
                len = sizeof(command);  
                ret = recv(tmp_sk, &command, len, MSG_WAITALL);
                if(ret == -1) {
                        cerr << "Errore: funzione <recv> sul comando non eseguita correttamente" << endl;
                        pthread_exit(NULL);
                }
                if(ret == 0) {
                        cout << "Chiusa la connessione con il client-socket: " << tmp_sk << endl;
                        pthread_exit(NULL);
                }

                if(command == 'm') {
                        ret = serv.block_map(tmp_sk);
                        if(ret == -1) {
                                cerr << "Errore: funzione <block_map> non eseguita correttamente" << endl;
                                pthread_exit(NULL);             
                        }       
                }

                if(command == 'u') {
                        ret = serv.block_unmap(tmp_sk);
                        if(ret == -1) {
                                cerr << "Errore: funzione <block_unmap> non eseguita correttamente" << endl;
                                pthread_exit(NULL);             
                        }       
                }

                if(command == 'p') {                            
                        ret = serv.block_update(tmp_sk);
                        if(ret == -1) {
                                cerr << "Errore: funzione <block_update> non eseguita correttamente" << endl;
                                pthread_exit(NULL);             
                        }       
                }

                if(command == 'w') {
                        ret = serv.block_write(tmp_sk);
                        if(ret == -1) {
                                cerr << "Errore: funzione <block_write> non eseguita correttamente" << endl;
                                pthread_exit(NULL);             
                        }       
                }

                if(command == 's') {
                        ret = serv.block_wait(tmp_sk);
                        if(ret == -1) {
                                cerr << "Errore: funzione <block_wait> non eseguita correttamente" << endl;
                                pthread_exit(NULL);             
                        }       
                }
        }

        return NULL;
}

int main(int argc, char *argv[])
{               
        Server sv;
        argument arg;

        /* Variabili e strutture standard per la gestione della connessione */
        int sv_sk;
        int sv_sk_act;
        int sv_ret;
        socklen_t addrlen;
        struct sockaddr_in my_addr;
        struct sockaddr_in cl_addr;

        /* Inizializzazione del semaforo di mutua esclusione */ 
        pthread_mutex_init(&m_acc, NULL);

        pthread_t thread_id;
        int ptc;

        if(argc != 4) {
                cerr << "Errore: sintassi del comando errata" << endl;
                cout << "Digitare: ./server <indirizzo IP> <porta> <path del configfile>" << endl;
                exit(1);
        }       
        
        SERVER_IP_ADD = argv[1];
        SERVER_PORT = atoi(argv[2]);

        sv.port_control(SERVER_PORT);
        
        sv_sk = socket(AF_INET, SOCK_STREAM, 0);
        if(sv_sk == -1) {
                cerr << "Errore: funzione <socket> non eseguita correttamente sul server" << endl;
                exit(0);
        }

        /* Valore standard usato nella <setsockopt> */
        const int on = 1; 

        /* 
        Opzione <SO_REUSEADDR>: fa il restart del server se si effettua una bind su una certa porta
        quando sono presenti delle connnessioni established che usano la suddetta porta 
        */
        sv_ret = setsockopt(sv_sk, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        if(sv_ret == -1) {
                cerr << "Errore: impossibile settare l'opzione <SO_REUSEADDR> sul server" << endl;
                exit(0);
        }

        /* Inizializzazione struttura dati */
        bzero(&my_addr, sizeof(struct sockaddr_in)); /* Azzera il contenuto della struttura */
        my_addr.sin_family = AF_INET;
        my_addr.sin_port = htons(SERVER_PORT);
        inet_pton(AF_INET, SERVER_IP_ADD, &my_addr.sin_addr.s_addr); /* Converte l'indirizzo da stringa a valore numerico */

        sv_ret = bind(sv_sk, (struct sockaddr *)(&my_addr), sizeof(my_addr));
        if(sv_ret == -1) {
                cerr << "Errore: funzione <bind> non eseguita correttamente sul server" << endl;
                exit(0);
        }

        config_file_path = argv[3];
        sv.read_config_file(config_file_path);
        
        sv_ret = listen(sv_sk, BACKLOG);
        if(sv_ret == -1) {
                cerr << "Errore: funzione <listen> non eseguita correttamente sul server" << endl;
                exit(0);
        }  

        cout << endl;
        cout << "SERVER IN ASCOLTO" << endl;
        cout << "Indirizzo IP server: " << SERVER_IP_ADD << endl;
        cout << "Porta server: " << SERVER_PORT << endl;
        cout << "Descrittore del soket lato server in ascolto: " << sv_sk << endl;      
        cout << endl;                   

        while(1) {
                addrlen = sizeof(cl_addr);
                sv_sk_act = accept(sv_sk, (struct sockaddr *)(&cl_addr), &addrlen);
                if(sv_sk_act == -1) {
                        cerr << "Errore: funzione <accept> non eseguita correttamente sul server" << endl;
                        exit(0);
                } 

                cout << "Aperta la connessione con il client-socket: " << sv_sk_act << endl;
                
                pthread_mutex_lock(&m_acc);

                arg.s = sv;
                arg.use_sk = sv_sk_act;

                ptc = pthread_create(&thread_id, 0, thread_body, (void *)&arg);
                if(ptc) {
                        cerr << "Errore: funzione <pthread_create> non eseguita correttamente" << endl;
                        exit(-1);               
                }
        }

        close(sv_sk);
        
        return 0;
}

// End test4
