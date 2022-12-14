#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define REFERENCE_TIME 300000
#define PAGES_NUMBER_FLOOR 1
#define PAGES_NUMBER_CEILING 1200
#define RANDOM_WINDOW_FLOOR 1
#define RANDOM_WINDOW_CEILING 20
#define LOCALITY_WINDOW_FLOOR 1000
#define LOCALITY_WINDOW_CEILING 2500

typedef struct frame {
    int value;
    int referenced;
    int dirty;
    int info; // for ARB
    struct frame *next;
} Frame;

typedef struct frameList {
    int size;
    Frame *head;
    Frame *tail;
} FrameList;

FrameList *createFrameList() {
    FrameList *frameList = (FrameList*)malloc(sizeof(FrameList));
    frameList -> size = 0;
    frameList -> head = NULL;
    frameList -> tail = NULL;

    return frameList;
}

void processReferenceStringWithRandom(int *referenceString) {
    int currentIndex = 0;
    int isStillProcessing = 1;

    while(isStillProcessing) {
        int windowSize = rand() % (RANDOM_WINDOW_CEILING - RANDOM_WINDOW_FLOOR + 1) + RANDOM_WINDOW_FLOOR;
        int startNumber = rand() % (PAGES_NUMBER_CEILING - PAGES_NUMBER_FLOOR + 1) + PAGES_NUMBER_FLOOR;

        for(int i = 0; i < windowSize; i++) {
            referenceString[currentIndex] = startNumber;
            startNumber += 1;

            if(startNumber == PAGES_NUMBER_CEILING + 1) {
                startNumber = 1;
            }

            currentIndex += 1;
            if(currentIndex == REFERENCE_TIME) {
                isStillProcessing = 0;
                break;
            }
        }
    }
}

void processReferenceStringWithLocality(int *referenceString) {
    int currentIndex = 0;
    int innerWindowSize = 5;
    int pickedNumber = 0;
    int isStillProcessing = 1;
    
    while(isStillProcessing) { 
        int windowSize = rand() % (LOCALITY_WINDOW_CEILING - LOCALITY_WINDOW_FLOOR + 1) + LOCALITY_WINDOW_FLOOR;
        int startNumber = rand() % (PAGES_NUMBER_CEILING - PAGES_NUMBER_FLOOR + 1) + PAGES_NUMBER_FLOOR;

        for(int i = 0; i < windowSize; i++) {
            // startNumber > (PAGES_NUMBER_CEILING - innerWindowSize + 1) || startNumber < (PAGES_NUMBER_FLOOR + innerWindowSize - 1)
            if(startNumber < (PAGES_NUMBER_FLOOR + innerWindowSize - 1)) {
                int temporaryPicked[2];
                int randomIndex = rand() % 2;

                temporaryPicked[0] = rand() % (PAGES_NUMBER_CEILING - startNumber + 1) + startNumber;
                temporaryPicked[1] = rand() % ((PAGES_NUMBER_FLOOR + innerWindowSize - (PAGES_NUMBER_CEILING - startNumber + 1) - 1) - PAGES_NUMBER_FLOOR + 1) + PAGES_NUMBER_FLOOR;

                pickedNumber = temporaryPicked[randomIndex];
	    } else {
                pickedNumber = rand() % (startNumber - (startNumber - innerWindowSize + 1) + 1) + (startNumber - innerWindowSize + 1);
            }
	    
            referenceString[currentIndex] = pickedNumber;
            currentIndex += 1;

            if(currentIndex > REFERENCE_TIME) {
                isStillProcessing = 0;
                break;
            }
        }
    }
}

void processReferenceStringWithSequenceAndJump(int *referenceString) {
    int isStillProcessing = 1;
    int currentIndex = 0;

    // ?????????????????? 5 ~ 10
    int smallWindowFloor = 5;
    int smallWindowCeiling = 10;
    // ?????? references ???????????? 50 ~ 100
    int bigWindowFloor = 50;
    int bigWindowCeiling = 100;

    while(isStillProcessing) {
        // ??????????????????random 1 ~ 10
        int jumpingProbability = rand() % 10 + 1;
        int windowSize = 0;
        int startNumber; // ????????????????????? startNumber

        // ???????????? 80% ??????????????????
        if(jumpingProbability > 2) {
            int isJumpingFar = 0;
            int temporaryStartNumber;
            int temporaryFloor;
            int temporaryCeiling;

            windowSize = rand() % (smallWindowCeiling - smallWindowFloor + 1) + smallWindowFloor;

            while(!isJumpingFar) {
                temporaryStartNumber = rand() % ((PAGES_NUMBER_CEILING - windowSize + 1) - PAGES_NUMBER_FLOOR + 1) + PAGES_NUMBER_FLOOR;
                temporaryFloor = temporaryStartNumber - 100;
                temporaryCeiling = temporaryStartNumber + 100;

                // ????????????
                if(temporaryFloor < PAGES_NUMBER_FLOOR || temporaryCeiling > PAGES_NUMBER_CEILING) {
                    continue;
                }

                // ?????????????????? startNumber ??? +-100 ?????????????????????????????????????????????
                if(!((startNumber > temporaryStartNumber - 100) && (startNumber < temporaryStartNumber + 100))) {
                    startNumber = temporaryStartNumber;
                    isJumpingFar = 1;
                }
            }
        } else {
            windowSize = rand() % (bigWindowCeiling - bigWindowFloor + 1) + bigWindowFloor;
            startNumber = rand() % ((PAGES_NUMBER_CEILING - windowSize + 1) - PAGES_NUMBER_FLOOR + 1) + PAGES_NUMBER_FLOOR;
        }

        for(int i = 0; i < windowSize; i++) {
            // ????????? reference ??????????????? (n, n+1, n+2...)
            referenceString[currentIndex] = startNumber;
            startNumber += 1;

            currentIndex += 1;
            if(currentIndex == REFERENCE_TIME) {
                isStillProcessing = 0;
                break;
            }
        }
    }
}

void printReferenceString(int *referenceString) {
    for(int i = 0; i < REFERENCE_TIME; i++) {
        printf("%d ", referenceString[i]);
    }
}

// ?????? reference string ?????????????????????
void checkReferenceString(int *referenceString) {
    for(int i = 0; i < REFERENCE_TIME; i++) {
        if(referenceString[i] < PAGES_NUMBER_FLOOR || referenceString[i] > PAGES_NUMBER_CEILING) {
            printf("%d", referenceString[i]);
            printf("\nReference String is out-of-bounds.\n");
        }
    }
}

void printFrames(FrameList *frameList) {
    Frame *temporaryFrame = frameList -> head;
    
    while(temporaryFrame != NULL) {
        printf("%d ", temporaryFrame -> value);
        temporaryFrame = temporaryFrame -> next;
    }

    free(temporaryFrame);
    printf("\n");
}

void freeAllTheFrames(FrameList* frameList) {
    Frame *frame = frameList -> head;
    Frame *temporaryFrame;
    
    while(frame != NULL) {
        temporaryFrame = frame;
        frame = frame -> next;
        free(temporaryFrame);
    }

    free(frameList);
}

void pageReplacementWithFIFO(int *referenceString, int frameSize, int *result) {
    int pageFaultCount = 0;
    int interruptCount = 0;
    int diskWriteCount = 0;

    FrameList *frameList = createFrameList();

    for(int i = 0; i < REFERENCE_TIME; i++) {
        Frame *temporaryFrame = frameList -> head;
        int hit = 0;

        while(temporaryFrame != NULL) {
            // ??????????????? frame ??? (hit)
            if(referenceString[i] == temporaryFrame -> value) {
                hit = 1;
                break;
            }
            temporaryFrame = temporaryFrame -> next;
        }

        // hit ????????? page fault
        if(hit) {
            continue;
        }

        free(temporaryFrame);
        // ????????? frame ??? (miss)
        Frame *frame = (Frame*)malloc(sizeof(Frame));
        frame -> value = referenceString[i];
        frame -> next = NULL;

        // ?????? dirty bit????????? 50/50???
        int isDirty = rand() % 2;
        
        frame -> dirty = isDirty;

        // ???????????????
        if(frameList -> head == NULL) {
            frameList -> head = frame;
            frameList -> size += 1;
        } else {
            // Frame *temporaryFrame = frameList -> head;            
            frameList -> tail -> next = frame;

            // frame ????????????
            if(frameList -> size < frameSize) {
                frameList -> size += 1;
            // frame ????????????????????????
            } else {
                Frame *firstFrame = frameList -> head;
                frameList -> head = firstFrame -> next;
                free(firstFrame);

                // ?????? disk write
                if(frame -> dirty) {
                    frame -> dirty = 0;
                    diskWriteCount += 1;
                }
            }
        }

        frameList -> tail = frame;
        // ????????? frame ????????? replacement ??????page fault +1
        pageFaultCount += 1;
        // printFrames(frameList);
    }

    freeAllTheFrames(frameList);

    result[0] = pageFaultCount;
    result[1] = interruptCount;
    result[2] = diskWriteCount;
}

void pageReplacementWithOptimal(int *referenceString, int frameSize, int *result) {
    int pageFaultCount = 0;
    int interruptCount = 0;
    int diskWriteCount = 0;

    FrameList *frameList = createFrameList();

    for(int i = 0; i < REFERENCE_TIME; i++) {
        Frame *temporaryFrame = frameList -> head;
        int hit = 0;

        while(temporaryFrame != NULL) {
            // ??????????????? frame ??? (hit)
            if(referenceString[i] == temporaryFrame -> value) {
                hit = 1;
                break;
            }
            temporaryFrame = temporaryFrame -> next;
        }

        // hit ????????? page fault
        if(hit) {
            continue;
        }

        free(temporaryFrame);
        // ????????? frame ??? (miss)
        Frame *frame = (Frame*)malloc(sizeof(Frame));
        frame -> value = referenceString[i];
        frame -> next = NULL;

        // ?????? dirty bit????????? 50/50???
        int isDirty = rand() % 2;
        
        frame -> dirty = isDirty;

        // ???????????????
        if(frameList -> head == NULL) {
            frameList -> head = frame;
            frameList -> size += 1;
            frameList -> tail = frame;
        } else {
            // frame ????????????
            if(frameList -> size < frameSize) {
                frameList -> size += 1;
                frameList -> tail -> next = frame;
                frameList -> tail = frame;
            // frame ????????????????????????
            } else {
                // OS ?????????????????????????????? reference string
                interruptCount += 1;

                // ???????????????????????? frame ????????????????????????????????? reference ???
                Frame *currentFrame = frameList -> head;
                int currentFrameIndex = 0;
                int farthestIndex = 0;
                int farthestFrameIndex = 0;

                while(currentFrame != NULL) {
                    int willNotReferenced = 1;

                    // ??? reference string ?????????
                    for(int j = i; j < REFERENCE_TIME; j++) {
                        if(referenceString[j] == currentFrame -> value) {
                            // ??????????????????????????????????????????????????????????????? frame
                            if(j > farthestIndex) {
                                farthestIndex = j;
                                farthestFrameIndex = currentFrameIndex;
                            }
                            willNotReferenced = 0;
                            // ????????????????????????
                            break;
                        }
                    }

                    // ????????????????????????????????? frame ??????
                    if(willNotReferenced) {
                        farthestFrameIndex = currentFrameIndex;
                        // ????????????????????????????????? locality???
                        break;
                    }

                    currentFrame = currentFrame -> next;
                    currentFrameIndex += 1;
                }

                // ??????????????? head ???
                if(farthestFrameIndex == 0) {
                    Frame *temporaryFrame = frameList -> head;

                    frame -> next = temporaryFrame -> next;
                    frameList -> head = frame;
                    free(temporaryFrame);
                // ??????????????? tail ???
                } else if(farthestFrameIndex == frameSize - 1){
                    Frame *temporaryFrame = frameList -> tail;
                    Frame *beforeTail = frameList -> head;

                    // ?????? tail ?????????
                    for(int k = 0; k < frameSize - 2; k++) {
                        beforeTail = beforeTail -> next;
                    }

                    beforeTail -> next = frame;
                    frameList -> tail = frame;
                    free(temporaryFrame);
                } else {
                    // ??? head ???????????????
                    Frame *temporaryFrame = frameList -> head;
                    Frame *previousFrame = frameList -> head;
                    Frame *targetFrame;
                    Frame *nextFrame;
                    
                    // ????????? frame ???????????????
                    for(int x = 0; x < farthestFrameIndex; x++) {
                        previousFrame = temporaryFrame;
                        targetFrame = previousFrame -> next;
                        nextFrame = targetFrame -> next;
                        temporaryFrame = temporaryFrame -> next;
                    }

                    // ????????? frame
                    previousFrame -> next = frame;
                    frame -> next = nextFrame;
                    free(targetFrame);
                }
                // ?????? disk write
                if(frame -> dirty) {
                    frame -> dirty = 0;
                    diskWriteCount += 1;
                }
            }
        }
        // printFrames(frameList);
        pageFaultCount += 1;
    }

    result[0] = pageFaultCount;
    result[1] = interruptCount;
    result[2] = diskWriteCount;

    freeAllTheFrames(frameList);
}

void pageReplacementWithARB(int *referenceString, int frameSize, int *result) {
    // ?????????????????? gcc ??????
    // https://gcc.gnu.org/onlinedocs/gcc/Binary-constants.html
    // ???????????? 8 bits
    // ????????? 11111111 = (decimal) 255

    int pageFaultCount = 0;
    int interruptCount = 0;
    int diskWriteCount = 0;

    FrameList *frameList = createFrameList();

    for(int i = 0; i < REFERENCE_TIME; i++) {
        Frame *temporaryFrame = frameList -> head;
        int hit = 0;

        while(temporaryFrame != NULL) {
            // ??????????????? frame ??? (hit)
            if(referenceString[i] == temporaryFrame -> value) {
                // OS ????????????????????? info
                interruptCount += 1;

                // leftmost ?????? 1
                temporaryFrame -> info += 128;

                hit = 1;
                break;
            }
            temporaryFrame = temporaryFrame -> next;
        }

        // hit ????????? page fault
        if(hit) {
            continue;
        }

        // ?????? 8 ????????????????????? shift
        // ?????? shift 1 ???
        if(i % 8 == 0 && i != 0) {
            // OS ????????????????????? shift
            interruptCount += 1;

            Frame *currentFrame = frameList -> head;

            while(currentFrame != NULL) {
                currentFrame -> info >>= 1;
                currentFrame = currentFrame -> next;
            }

            free(currentFrame);
        }

        free(temporaryFrame);
        // ????????? frame ??? (miss)
        Frame *frame = (Frame*)malloc(sizeof(Frame));
        frame -> value = referenceString[i];
        frame -> next = NULL;

        // ?????? dirty bit????????? 50/50???
        int isDirty = rand() % 2;
        
        frame -> dirty = isDirty;

        // ???????????????
        if(frameList -> head == NULL) {
            frame -> info = 0b00000000;
            frameList -> head = frame;
            frameList -> size += 1;
            frameList -> tail = frame;
        } else {
            // frame ????????????
            if(frameList -> size < frameSize) {
                frame -> info = 0b00000000;
                frameList -> size += 1;
                frameList -> tail -> next = frame;
                frameList -> tail = frame;
            // frame ????????????????????????
            } else {
                // ?????????????????? leftmost ??? 1
                frame -> info = 0b10000000;

                Frame *currentFrame = frameList -> head;
                Frame *targetFrame = currentFrame;
                int largestInfo = 0b11111111;

                while(currentFrame != NULL) {
                    if(currentFrame -> info <= largestInfo) {
                        targetFrame = currentFrame;
                        largestInfo = currentFrame -> info;
                    }

                    currentFrame = currentFrame -> next;
                }

                // ??????????????? head ???
                if(targetFrame == frameList -> head) {
                    frame -> next = targetFrame -> next;
                    frameList -> head = frame;
                // ??????????????? tail ????????????
                } else {
                    Frame *previousFrame = frameList -> head;
                    Frame *nextFrame;

                    while(previousFrame -> next != targetFrame) {
                        previousFrame = previousFrame -> next;
                    }

                    // ??????????????? tail ???
                    if(previousFrame -> next -> next == NULL) {
                        previousFrame -> next = frame;
                        frameList -> tail = frame;
                    // ????????????????????????
                    } else {
                        targetFrame = previousFrame -> next;
                        nextFrame = targetFrame -> next;

                        previousFrame -> next = frame;
                        frame -> next = nextFrame;

                        free(targetFrame);
                    }
                }

                // ?????? disk write
                if(frame -> dirty) {
                    frame -> dirty = 0;
                    diskWriteCount += 1;
                }
            }
        }
        pageFaultCount += 1;
    }

    result[0] = pageFaultCount;
    result[1] = interruptCount;
    result[2] = diskWriteCount;

    freeAllTheFrames(frameList);
}

void pageReplacementWithDistance(int *referenceString, int frameSize, int *result) {
    int pageFaultCount = 0;
    int interruptCount = 0;
    int diskWriteCount = 0;

    FrameList *frameList = createFrameList();

    for(int i = 0; i < REFERENCE_TIME; i++) {
        Frame *temporaryFrame = frameList -> head;
        int hit = 0;

        while(temporaryFrame != NULL) {
            // ??????????????? frame ??? (hit)
            if(referenceString[i] == temporaryFrame -> value) {
                hit = 1;
                break;
            }
            temporaryFrame = temporaryFrame -> next;
        }

        // hit ????????? page fault
        if(hit) {
            continue;
        }

        free(temporaryFrame);
        // ????????? frame ??? (miss)
        Frame *frame = (Frame*)malloc(sizeof(Frame));
        frame -> value = referenceString[i];
        frame -> next = NULL;

        // ?????? dirty bit????????? 50/50???
        int isDirty = rand() % 2;
        
        frame -> dirty = isDirty;

        // ???????????????
        if(frameList -> head == NULL) {
            frameList -> head = frame;
            frameList -> size += 1;
            frameList -> tail = frame;
        } else {
            // frame ????????????
            if(frameList -> size < frameSize) {
                frameList -> size += 1;
                frameList -> tail -> next = frame;
                frameList -> tail = frame;
            // frame ????????????????????????
            } else {
                // OS ???????????????????????????
                interruptCount += 1;

                // ????????????
                int longestDistance = PAGES_NUMBER_FLOOR;
                Frame *calculatingFrame = frameList -> head;
                Frame *longestDistanceFrame = frameList -> head;

                for(int j = 0; j < frameSize; j++) {
                    int clockWiseDistance = 0;
                    int counterClockWiseDistance = 0;
                    int currentFrameLongestDistance;


                    if(frame -> value > calculatingFrame -> value) {
                        clockWiseDistance = (PAGES_NUMBER_CEILING - frame -> value) + 1 +(calculatingFrame -> value - PAGES_NUMBER_FLOOR);
                        counterClockWiseDistance = frame -> value - calculatingFrame -> value;
                    } else {
                        clockWiseDistance = (PAGES_NUMBER_CEILING - calculatingFrame -> value) + 1 + (frame -> value - PAGES_NUMBER_FLOOR);
                        counterClockWiseDistance = calculatingFrame -> value - frame -> value;
                    }

                    // ?????????????????????????????????????????????
                    if(clockWiseDistance < counterClockWiseDistance) {
                        currentFrameLongestDistance = clockWiseDistance;
                    } else {
                        currentFrameLongestDistance = counterClockWiseDistance;
                    }

                    // ????????????????????????
                    if(currentFrameLongestDistance > longestDistance) {
                        longestDistance = currentFrameLongestDistance;
                        longestDistanceFrame = calculatingFrame;
                    } else if(currentFrameLongestDistance == longestDistance) {
                        if(calculatingFrame -> value < longestDistanceFrame -> value) {
                            longestDistanceFrame = calculatingFrame;
                        }
                    }

                    calculatingFrame = calculatingFrame -> next;
                }

                // ????????????????????? frame
                // ?????? head
                if(longestDistanceFrame == frameList -> head) {
                    Frame *temporaryFrame = frameList -> head;

                    frame -> next = temporaryFrame -> next;
                    frameList -> head = frame;

                    free(temporaryFrame);
                // ??????????????? tail ????????????
                } else {
                    Frame *previousFrame = frameList -> head;
                    Frame *targetFrame;
                    Frame *nextFrame;

                    while(previousFrame -> next != longestDistanceFrame) {
                        previousFrame = previousFrame -> next;
                    }

                    // ??????????????? tail ???
                    if(previousFrame -> next -> next == NULL) {
                        previousFrame -> next = frame;
                        frameList -> tail = frame;
                    // ????????????????????????
                    } else {
                        targetFrame = previousFrame -> next;
                        nextFrame = targetFrame -> next;

                        previousFrame -> next = frame;
                        frame -> next = nextFrame;

                        free(targetFrame);
                    }
                }

                // ?????? disk write
                if(frame -> dirty) {
                    frame -> dirty = 0;
                    diskWriteCount += 1;
                }
            }
        }
        pageFaultCount += 1;
    }

    result[0] = pageFaultCount;
    result[1] = interruptCount;
    result[2] = diskWriteCount;

    freeAllTheFrames(frameList);
}

void readFileAsReferenceString(int *referenceString){
    FILE *file;
    file = fopen("random.txt", "r");

    for(int i = 0; i < REFERENCE_TIME; i++) {
        fscanf(file, "%d, ", &referenceString[i]);
    }

    fclose(file);
}

void generateResultForConsole(int *referenceString, int *frameSize) {
    int results[3];

    for(int i = 0; i < 3; i++) {
        switch(i) {
            case 0:
                // ?????? Random
                printf("###################################################\n");
                printf("Using -> Random\n");
                processReferenceStringWithRandom(referenceString);
                checkReferenceString(referenceString);
                printf("###################################################\n\n");
                
                break;
            case 1:
                // ?????? Locality
                printf("###################################################\n");
                printf("Using -> Locality\n");
                processReferenceStringWithLocality(referenceString);
                checkReferenceString(referenceString);
                printf("###################################################\n\n");
                
                break;
            case 2:
                // ?????? Sequence And Jump
                printf("###################################################\n");
                printf("Using -> Sequence And Jump\n");
                processReferenceStringWithSequenceAndJump(referenceString);
                checkReferenceString(referenceString);
                printf("###################################################\n\n");
                
                break;
        }

        for(int j = 0; j < 5; j++) {
            printf("Frame Size: [%d]\n", frameSize[j]);
            printf("---------------------------------------------------\n");
            printf("Algorithm   Page Faults   Interrupts    Disk Writes\n");
            printf("---------------------------------------------------");
            pageReplacementWithFIFO(referenceString, frameSize[j], results);
            printf("\nFIFO     ");
            printf("        %6d", results[0]);
            printf("       %6d", results[1]);
            printf("         %6d", results[2]);
            pageReplacementWithOptimal(referenceString, frameSize[j], results);
            printf("\nOptimal  ");
            printf("        %6d", results[0]);
            printf("       %6d", results[1]);
            printf("         %6d", results[2]);
            pageReplacementWithARB(referenceString, frameSize[j], results);
            printf("\nARB      ");
            printf("        %6d", results[0]);
            printf("       %6d", results[1]);
            printf("         %6d", results[2]);
            pageReplacementWithDistance(referenceString, frameSize[j], results);
            printf("\nDistance ");
            printf("        %6d", results[0]);
            printf("       %6d", results[1]);
            printf("         %6d", results[2]);
            printf("\n\n");
        }
    }
}

void generateResultForGraph(int *referenceString, int *frameSize) {
    // ???????????????
    // Page Faults -> Interrupts -> Disk Writes
    // Random -> Locality -> Sequence And Jump
    // (Frame Size, FIFO, Optimal, ARB, Distance)
    // ???????????? ./output.txt

    // ????????????
    int results[5][12][3];

    // FIFO
    int randomWithFIFO[3];
    int localityWithFIFO[3];
    int sequenceAndJumpWithFIFO[3];

    // Optimal
    int randomWithOptimal[3];
    int localityWithOptimal[3];
    int sequenceAndJumpWithOptimal[3];

    // ARB
    int randomWithARB[3];
    int localityWithARB[3];
    int sequenceAndJumpWithARB[3];

    // Distance
    int randomWithDistance[3];
    int localityWithDistance[3];
    int sequenceAndJumpWithDistance[3];

    processReferenceStringWithRandom(referenceString);
    for(int i = 0; i < 5; i++) {
        pageReplacementWithFIFO(referenceString, frameSize[i], randomWithFIFO);
        pageReplacementWithOptimal(referenceString, frameSize[i], randomWithOptimal);
        pageReplacementWithARB(referenceString, frameSize[i], randomWithARB);
        pageReplacementWithDistance(referenceString, frameSize[i], randomWithDistance);

        for(int j = 0; j < 3; j++) {
            results[i][0][j] = randomWithFIFO[j];
            results[i][1][j] = randomWithOptimal[j];
            results[i][2][j] = randomWithARB[j];
            results[i][3][j] = randomWithDistance[j];
        }
    }

    processReferenceStringWithLocality(referenceString);
    for(int i = 0; i < 5; i++) {
        pageReplacementWithFIFO(referenceString, frameSize[i], localityWithFIFO);
        pageReplacementWithOptimal(referenceString, frameSize[i], localityWithOptimal);
        pageReplacementWithARB(referenceString, frameSize[i], localityWithARB);
        pageReplacementWithDistance(referenceString, frameSize[i], localityWithDistance);

        for(int j = 0; j < 3; j++) {
            results[i][4][j] = localityWithFIFO[j];
            results[i][5][j] = localityWithOptimal[j];
            results[i][6][j] = localityWithARB[j];
            results[i][7][j] = localityWithDistance[j];
        }
    }

    processReferenceStringWithSequenceAndJump(referenceString);
    for(int i = 0; i < 5; i++) {
        pageReplacementWithFIFO(referenceString, frameSize[i], sequenceAndJumpWithFIFO);
        pageReplacementWithOptimal(referenceString, frameSize[i], sequenceAndJumpWithOptimal);
        pageReplacementWithARB(referenceString, frameSize[i], sequenceAndJumpWithARB);
        pageReplacementWithDistance(referenceString, frameSize[i], sequenceAndJumpWithDistance);

        for(int j = 0; j < 3; j++) {
            results[i][8][j] = sequenceAndJumpWithFIFO[j];
            results[i][9][j] = sequenceAndJumpWithOptimal[j];
            results[i][10][j] = sequenceAndJumpWithARB[j];
            results[i][11][j] = sequenceAndJumpWithDistance[j];
        }
    }

    FILE *file;
    file = fopen("output.txt", "w");

    // Page Faults -> Interrupts -> Disk Writes
    for(int i = 0; i < 3; i++) {
        // Frame Sizes: 30 -> 60 -> 80 -> 120 -> 150
        for(int x = 0; x < 5; x++) {
            // Random -> Locality -> Sequence And Jump
            for(int y = 0; y < 12; y++){
                fprintf(file, "%d ", frameSize[x]);
                fprintf(file, "%d\n", results[x][y][i]);
            }
        }
    }

    fclose(file);
}

int main() {
    int referenceString[REFERENCE_TIME];
    int frameSize[5] = {30, 60, 90, 120, 150};

    srand(time(NULL));
    
    generateResultForConsole(referenceString, frameSize);
    // generateResultForGraph(referenceString, frameSize);
    
    printf("\n");

    return 0;
}
