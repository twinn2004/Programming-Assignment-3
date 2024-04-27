/*
Taylor Winn
Programming Assignment 3
March 5th, 2024
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_NUM 500000  // const for the greatest amout of tickets a person can purchase
#define MAX_QUEUES 12   // const for the max number of POSSIBLE queues

typedef struct Customer{    // struct to store each customer's information
    char name[51]; 
    int ticketnum;
    int arrival;
    int origQ;  
} Customer;

typedef struct Node{    // linked list sturct to implement the other sturcts
    struct Node * next;
    Customer * value;
} Node;

typedef struct Queue{   // queue struct to be implemented as a linked list 
    Node * front;
    Node * tail;
    int Qsize;
    bool initial;   // the booth initial condition checks to make sure that a queue has been initialized (returned true is so, false if not)
} Queue;

typedef struct Booth{   // booth struct, stores values for each individual booth (booth = i+1)
    int queues[13];
    int numqueue;   // number of queues within the booth
} Booth;

int main(int argc, char *argv[]){

    Customer * createCustomer(const char *name, int ticketnum, int arrival);
    
    Queue *newQ();
    void NQ(Queue * queue, Customer *customer);
    Customer *DQ(Queue *queue);
    Customer *Peek(Queue *queue);
    int IsEmpty(Queue *queue);
    
    void BoothAssignment(Booth booths[], int numBooths, int numqueues, int ValidQueues[], Queue queues[]);
    void MergeQueues(Booth booths[], int numBooths, Queue queues[]);
    void SortQueues(Booth booths[], Queue queues[], int numBooths);
    void printInfo(Booth booths[], Queue queues[], int numBooths);

    FILE *file = NULL;  // file pointer
    file = fopen(argv[1], "r"); // command line argument for file
    if(file == NULL){       
        printf("cannot open file"); // tedious condition just in case
        return 1;
    } 
    Queue queues[MAX_QUEUES];   // creating the initial array to store all queues 
    memset(queues, 0, sizeof(queues));  // using memset to initialize each index within the array to 0

    int numPeople;
    int numBooths;

    fscanf(file, "%d %d", &numPeople, &numBooths);   // using fscanf to scan the file for input and assign it to the appropriate variables

    Booth booths[numBooths];    // creating array for the number of booths
    memset(booths, 0, sizeof(booths));  // again using memeset to initialize all values within to 0

    int numqueues = 0;  
    for(int i = 0; i < numPeople; i++){
        char buffer[51];    // buffer to scan in the name and dynamically allocate the memeory within the customer array
        int hold1, hold2;   

        fscanf(file, "%s %d %d", buffer, &hold1, &hold2);

        Customer *newCustomer = createCustomer(buffer, hold1, hold2);   // calling createcustomer function to assign memory / values within the customer array index
        char letter = buffer[0];    // retrieving the first letter of the person's name
        int q = letter - 'A';   // calculating ascii value
        int QueueNum = q%13;    // calculation to determine their appropraite queue
        if(QueueNum != 0){  // for the people who are in a defined queue
            if(queues[QueueNum - 1].initial){   // checks if their queue has already been initialized, and if so adds them to the backend (tail)
                Node * newNode = malloc(sizeof(Node));  // creating their node 
                if(newNode == NULL){
                    return -1;
                }
                newNode->value = newCustomer;   // the node is pointing to the customer struct values
                newNode->next = NULL;

                queues[QueueNum-1].tail->next = newNode;    // adding the new node to the tail of it's appropriate queue
                queues[QueueNum-1].tail = newNode;  
                queues[QueueNum-1].Qsize++; // incrementing the size of the queue (number of customers)
            } else {
                queues[QueueNum-1] = *newQ();   // if the person's routed queue has not been initialized, we need to create a new queue with them as the first element
                numqueues = numqueues + 1;  // incrimenting the number of queues 
                Node * newNode = malloc(sizeof(Node));
                if(newNode == NULL){
                    return -1;
                }
                newNode->value = newCustomer;
                newNode->next = NULL;

                queues[QueueNum-1].front = newNode; // since they're the only element for the time being, they are both front and tail
                queues[QueueNum-1].tail = newNode;
                queues[QueueNum-1].Qsize++;
                queues[QueueNum-1].initial = true;  // setting the bool value true for initial to indicate that the queue has been created and stores atleast 1 value
            }   
        } else {    // if the person's queue value is 0
            int index = -1;      
            for(int i = 0; i < MAX_QUEUES; i++){    // looping through the queues array
                if(queues[i].initial && queues[i].Qsize > 0){   // searching for the first initialized index that has atleast one person
                    index = i;
                    break;
                }
            }
            for(int i = index + 1; i < MAX_QUEUES; i++){
                if(queues[i].initial && queues[i].Qsize > 0){
                    if(queues[i].Qsize < queues[index].Qsize){  // for the checking to find which initialized queue contains the least number of people, but ha atleast 1
                        index = i;
                    }
                }
            }
            Node * newNode = malloc(sizeof(Node));
            if(newNode == NULL){
                return -1;
            }
            newNode->value = newCustomer;
            newNode->next = NULL;
            
            if(queues[index].front == NULL){
                queues[index].front = newNode;  // just making sure we have the correct value by ensuring that the front of the queue stores a value
            } else {
                queues[index].tail->next = newNode;
            }
            queues[index].tail = newNode;
            queues[index].Qsize++;
        }

    }   
    
    int *ValidQueues = malloc(numqueues * sizeof(int)); // determining how many queues have been initalized and therefore store values as they are the only queues we are concerned with from here out
    int count = 0;
    for(int i = 0; i < MAX_QUEUES; i++){
        if(queues[i].initial){
            ValidQueues[count] = i+1;
            count += 1;
        }
    }

    puts("");

    BoothAssignment(booths, numBooths, numqueues, ValidQueues, queues);
    MergeQueues(booths, numBooths, queues);
    
    for(int i = 0; i < numBooths; i++){ // having to split the size of each queue in hald because i was getting double the value for some reason and could not figure out why
        int size = queues[i].Qsize;
        queues[i].Qsize = size/2;
    }
    
    SortQueues(booths, queues, numBooths);
    printInfo(booths, queues, numBooths);
    
return 0;
}

Customer * createCustomer(const char *name, int ticketnum, int arrival){    // function to allocate space for a customer within the customer struct, as well as assign values
    Customer *newCostomer = malloc(sizeof(Customer));   // allocating space for the new person using malloc
    if(newCostomer == NULL){
        return NULL;
    } 
    strncpy(newCostomer->name, name, sizeof(newCostomer->name)-1);  // copying the name in the name value of the struct 
    newCostomer->ticketnum = ticketnum; // copying other values
    newCostomer->arrival = arrival;
    return newCostomer; // returns the newly created customer within the customer struct 
}

Queue *newQ(){  // function to create and initialize a queue 
    Queue * Q = malloc(sizeof(Queue));
    if(Q == NULL){
        return NULL;
    } 
    Q->front = NULL;
    Q->tail = NULL;
    Q->Qsize = 0;
    Q->initial = true;
    return Q;
}

void NQ(Queue * queue, Customer *customer){ // eneueue function, adds to a previously initialized queue
    Node *newNode = malloc(sizeof(Node));
    if(newNode == NULL){
        return;
    }

    newNode->value = customer;
    newNode->next = NULL;

    if(queue->front == NULL){
        queue->front = newNode;
    } else {
        queue->tail->next = newNode;
    }

    queue->tail = newNode;
    queue->Qsize += 1;
}

Customer *DQ(Queue *queue){ // dequeue function, removes and deallocates a node from a previously initialized queue
    if(queue == NULL || queue->front == NULL){
        return NULL;
    }

    Node *front = queue->front;
    Customer *customerNum = front->value;

    queue->front = front->next;
    free(front);    // freeing the node containing the value which is no longer needed

    if(queue->front == NULL){
        queue->tail = NULL;
    }

    queue->Qsize--; // decrementing the size for every removal 

    return customerNum;
}

Customer *Peek(Queue *queue){   // function to return the value stored at the front of the inputted queue
    if(queue == NULL || queue->front == NULL){
        return NULL;
    } else {
        return queue->front->value;
    }
}

int IsEmpty(Queue *queue){  // function to check if a queue is empty of contains an element
    if(queue->Qsize == 0){
        return 1;   // returns 1 if empty, 0 if there is atleast one element within
    } else {
        return 0;
    }
}

void BoothAssignment(Booth booths[], int numBooths, int numqueues, int ValidQueues[], Queue queues[]){  // function to assign the queues to their appropriate booths

    int QperB = numqueues/numBooths;    // determines the number of queues each booth will recieve as base
    int extraQ = numqueues%numBooths;   // determines the booths that will receive an extra queue (the first extraQ number of booths will have QperB + 1 queue)

    int Qindex = 0;
    int Bindex = 0;
    for(int i = 0; i < numBooths; i++){ // loooping through each booth
        booths[i].numqueue = QperB; // assigning the value numqueue within each booth to the initial number of queues each will receive
        if(i < extraQ){ // if the index is less than the number of booths receiving extra, add an extra accordingly
            booths[i].numqueue += 1;
        }
        for(int j = 0; j < booths[i].numqueue; j++){    // nested for loop to loop through the queues array 
            booths[i].queues[j] = ValidQueues[Qindex++];    // assigning the queue at index j to its appropriate booth at index i
        }
    }

}

void MergeQueues(Booth booths[], int numBooths, Queue queues[]){    // function to combine the queues within each booth into one large queue per booth (to simplify sorting later)
    
    for(int i = 0; i < numBooths; i++){
        Queue * FinalQ = newQ();    // creating a single new queue to replace the old multiple
        int size = 0;
        for(int k = 0; k < booths[i].numqueue; k++){
            int index = booths[i].queues[k] - 1;
            while(queues[index].front != NULL){
                Customer *customer = DQ(&queues[index]);    // calling the dequeue function in order to both retrieve the value of the person at the front of the queue, as well as free their spot in the orinigal (now uneeded) queue
                if(customer != NULL){
                    customer->origQ = index+1;
                    NQ(FinalQ, customer);   // calling enqueue function for the final queue
                    size += 1;  // incrimenting the size of the final queue
                }
            }
        }

        booths[i].numqueue = 1; // since we are merging all queues from each booth into one per booth, the numqueues per booth will only be one
        queues[i].Qsize = size; 

        Node *temp = FinalQ->front; 
        while(temp != NULL){
            NQ(&queues[i], temp->value);    // replacing the original queue with the new queue, in order to free the test queue
            temp = temp->next;
        }
        free(FinalQ);
    }
}

void SortQueues(Booth booths[], Queue queues[], int numBooths) {    // function to sort the values within each queue based on their 'arrival' value in the customer struct
    
    for (int i = 0; i < numBooths; i++) {   // looping through the booths 
        Queue *temp = &queues[i];
        if (temp != NULL && temp->front != NULL) {
            Node *index = temp->front;
            Customer *hold = NULL;
            Node *nextNode = NULL;
            
            
            while (index != NULL) {
                nextNode = index->next;
                while (nextNode != NULL) {
                    if (index->value->arrival > nextNode->value->arrival) { // if one arrival is greater than the hold variable, replace in order to move through from smallest to largest
                        hold = index->value;
                        index->value = nextNode->value;
                        nextNode->value = hold;
                    }
                    nextNode = nextNode->next;
                }
                
                index = index->next;
            }
        }
    }
}

void printInfo(Booth booths[], Queue queues[], int numBooths){  // function to print the output for each booth
    for(int i = 0; i < numBooths; i++){
        
        int prevExit = 0;   // hold variable for prevous exit, since in most cases the next person's entrance will depend on the previous person's exit
        printf("\nBooth %d", i+1);  // first print the booth value for which we are looping througn

        while(queues[i].front != NULL){ // loop that runs until are people are accounted for 
            Customer *customer = DQ(&queues[i]);    // freeing memory again using dequeue, as well as attaining the value
            int arrival;

            if(prevExit == 0 || (customer->arrival - prevExit) > 0) {   // checks to see if: A.) we are at the first person in the queue, B.) if the person arrived after the other person left, as those mean that the previous person's exit time is irrelevant
                arrival = customer->arrival;
            } else {
                arrival = prevExit; // if not, the current person's arrival time is equivilent to the person in front of them's exit time (fist in->fist out)
            }

            int pt = (5 * customer->ticketnum) + 30;    // variable storing the expression calculating each person's processing time
            int exitTime = arrival + pt;    // the exit time is just the time they arrived + the amount of time it takes for their order to be processed 
            printf("\n%s from line %d checks out at time: %d", customer->name, customer->origQ, exitTime);  // printing output
            
            prevExit = exitTime;    // assigning the current person's exit time to the previous value for the next ittereation

            free(customer); // freeing the space allocated for the person who is now finished at the booth from the customer struct
        }
        puts("");   // new booth
    }
}








