#include <iostream>
#include <vector>
#include <exception>
#include <functional>

using namespace std;

int P; // palya sugara
int N; // szamozott mezok szama


struct CoordXY{ // X-Y koordinata par
    int X,Y;
    CoordXY()=default;
    CoordXY(int pX, int pY) : X(pX), Y(pY) {};
};


struct CoordQR{ // Q-R koordinata par
    int Q,R;
    CoordQR()=default;
    CoordQR(int pQ, int pR) : Q(pQ), R(pR) {};
};


CoordXY QR2XY(CoordQR qr){ // Q-R transzformalasa X-Y-ra
    return CoordXY(qr.Q+P, qr.R+P);
}


CoordQR XY2QR(CoordXY xy){ // X-Y transzformalasa Q-R-re
    return CoordQR(xy.X-P, xy.Y-P);
}


struct Hexagon{
    CoordQR coord;
    bool isValid; // eleme-e ez a hexagon a palyanak, vagy csak a 2D-s tarolas miatt jott letre
    bool isOut; // lehet-e ez a mezo eleme a kornek, vagy mar kizartuk
    bool mustBeRoad; // resze-e ez a hexagon a vegso utnak
    bool isRoad; // eleme-e ez a hexagon az eppen felepitett utnak
    int roadCount; // hany szomszedos hexagonon kell, hogy legyen ut. -1 eseten barmennyin lehet
    static int nextId;
    int id;

    Hexagon() : isValid(true), isOut(false), mustBeRoad(false), isRoad(false), roadCount(-1),  id(nextId++) {};
};
int Hexagon::nextId=0;

enum class Direction {NORTHEAST, EAST, SOUTHEAST, SOUTHWEST, WEST, NORTHWEST}; // ilyen iranyu szomszedai lehetnek egy hexagonnak
const vector<Direction> directions={Direction::NORTHEAST, Direction::EAST, Direction::SOUTHEAST, Direction::SOUTHWEST, Direction::WEST, Direction::NORTHWEST}; // hogy konnyen vegigiteralhassunk az iranyokon


vector<vector<Hexagon>> hexGrid; // maga a palya, viszont ebben a 2D tarolas miatt lesznek ervenytelen mezok
vector<Hexagon*> houses; // pointert tarol a nem-nulla hazakra
vector<Hexagon*> zeroHouses; // pointereket tarol a nulla hazakra: invalidaljuk a kornyezetuket, majd bekenhagyjuk ezt a vektort
vector<int> houseRoadCounts; // az nemnulla hazak roadCount-jai (hany szomszedon kell, hogy ut legyen)
vector<vector<Hexagon*>> houseNeighbors; // a nemnulla hazak szomszedai hazankent


void collectHouseNeighbors(); // feltolti a houseRoadCounts es a houseNeighbors vektorokat


void buildHexGrid(); // megepiti a 2D-s hexagon vektort, valamint megjeloli azokat, amik valoban reszei a palyanak
void placeHouses();// elhelyezi a hazakat a palyan, vagyis beleirja mindegyik mezobe, hogy hany szomszedjan kell, hogy ut haladjon. Feltolti a houses vektort
void crossOutHexagons(); // meghivja a fv-eket, amik bizonyos szabalyok szerint kizarnak mezoket


void checkCorners(); // ha a sarokban 1-es van | sarok jobb/bal szomszedjan szam van | sarok kozepso szomszedjan 1-es vagy 2-es van
void checkOuterLayer(); // ha a kulso helyon van 1-es, akkor annak a ket szomszedja kizarva
void checkInnerLayer(); // ha a kulso helytol eggyel beljebb van 1-es, akkor annak a ket kulso hej szomszedja kizarva
void checkCloseOnes(); // ha ket 1-es egymas mellett van, vagy egy mezo van koztuk, akkor kizarhatunk mezoket
void checkZeros(); // invalidalja a 0-s hazak 1 sugaru kornyezetet a zeroHouses-on vegigiteralva
void findRoadPiece(); // a mezok kizarasa utan keres egy olyan mezot, ami biztosan resze kell, hogy legyen az utnak


Hexagon* getNeighbor(CoordQR coordQR, Direction dir); // visszaadja egy adott Q-R koordinataju mezo adott iranyu szomszedjat, ellenorizni kell, hogy nullptr-e+
vector<Direction> whereToGo(Direction from); // a from irany alapjan megadja, hogy melyik iranyokban mehetunk tovabb


Hexagon* start;
vector<Hexagon*> startNeighbors; // rajta biztosan megy ut, nezzuk meg, hogy innen merre lephetunk, ezekbe az iranyokba inditunk backtracket
vector<Direction> startDirections;
vector<Hexagon*> path; // ebbe tesszuk az ut elemeit
vector<Direction> chosenDirections; // milyen iranyokba lepkedtunk az eddig felepitett utban
vector<Direction> currentOptions; // egy adott ponton milyen iranyokban mehetunk tovabb

void fillStartNeighbors();

//******************************************* BACKTRACK functions *******************************************

struct nodeData{
    int copy_of_optionCount, copy_of_currentOption;
    vector<Direction> options;
};

nodeData currentNodeData;
vector<nodeData> pathNodes;

int controlVar,	// vezerlo valtozo
    decisionCount,	// korabbi dontesek szama
    optionCount,	// valasztasi lehetosegek szama
    currentOption;	// aktualis lehetoseg sorszama


bool optimalPath(){ // elertuk a celt?
	if(path.size()>5 && path.front()->id==path.back()->id){ //ha raleptunk a startmezore
        auto nextDirs=whereToGo(chosenDirections.back());
        for(auto d : nextDirs) if(d==chosenDirections[0]){ // es jo iranybol leptunk ra
            int counter;
            for(size_t i=0;i<houseNeighbors.size();i++){
                counter=0;
                for(auto house : houseNeighbors[i]) if(house->isRoad) counter++;
                if(counter!=houseRoadCounts[i]) return false;
            }
            return true;
        }
	}
	return false;
}

bool stillPossible(){ // lehetseges meg a cel elerese?
	int counter;
    for(size_t i=0;i<houseNeighbors.size();i++){
        counter=0;
        for(auto house : houseNeighbors[i]) if(house->isRoad) counter++;
        if(counter>houseRoadCounts[i]) return false;
    }
    return true;
}

int findOptions(){ // dontesi lehetosegek szambavetele
    currentOptions.clear();
    Hexagon* head=path.back();
    Hexagon* neighbor;
    vector<Direction> options=whereToGo(chosenDirections.back());
    for(Direction dir : options){
        neighbor=getNeighbor(head->coord,dir);
        if(neighbor!=nullptr && neighbor->isOut==false && neighbor->isRoad==false) currentOptions.push_back(dir);
    }
    return currentOptions.size();
}

void sortOptions(){ //dontesi lehetosegek rangsorolasa
}

void stepBack(){ // visszalepes es visszaallitas
    currentNodeData=pathNodes.back();
    pathNodes.pop_back();
    optionCount=currentNodeData.copy_of_optionCount;
    currentOption=currentNodeData.copy_of_currentOption;
    currentOptions=currentNodeData.options;
    Hexagon* lastHex=path.back();
    lastHex->isRoad=false;
    path.pop_back();
    chosenDirections.pop_back();
}

void save(){ //mentes
    currentNodeData.copy_of_optionCount=optionCount;
    currentNodeData.copy_of_currentOption=currentOption;
    currentNodeData.options=currentOptions;
    pathNodes.push_back(currentNodeData);
}

void stepForward(){ // elore lepes
    Hexagon* nextHex=getNeighbor(path.back()->coord,currentOptions[currentOption]);
    nextHex->isRoad=true;
    path.push_back(nextHex);
    chosenDirections.push_back(currentOptions[currentOption]);
}

void showResult(){ // megoldas kijelzese
    cout << path.size()-1 << endl;
    for(size_t i=1;i<path.size();i++) cout << path[i]->coord.Q << " " << path[i]->coord.R << endl;
}

//******************************************* EOF BACKTRACK functions *******************************************

int main () {
    cin >> P >> N;

    buildHexGrid();
    placeHouses();
    crossOutHexagons();
    collectHouseNeighbors();
    findRoadPiece();
    fillStartNeighbors();

    currentOptions.reserve(3);

    size_t shot=0;
    bool foundOptimal=false;
    while(!foundOptimal && shot<startNeighbors.size()){
        path.push_back(start);
        startNeighbors[shot]->isRoad=true;
        path.push_back(startNeighbors[shot]);
        chosenDirections.push_back(startDirections[shot]);
        pathNodes.clear();


        controlVar=1;
        decisionCount=0;
        while(controlVar){
            switch(controlVar){
                case 1: if (optimalPath()){
                            showResult();
                            controlVar=0;
                            foundOptimal=true; // leallunk
                          }
                          else if (stillPossible()) if ((optionCount=findOptions())){
                                              // sortOptions();
                                              currentOption=0;
                                              controlVar=3;
                                             }
                                             else controlVar=2;
                                  else controlVar=2;
                        break;
                case 2: if (decisionCount){
                            decisionCount--;
                            stepBack();
                            if (currentOption<optionCount-1){
                                currentOption++;
                                controlVar=3;
                               }
                               else controlVar=2;
                           }
                           else controlVar=0;
                        break;
                case 3: save();
                        stepForward();
                        decisionCount++;
                        controlVar=1;
            }
        }


        startNeighbors[shot]->isRoad=false;
        path.clear();
        chosenDirections.clear();
        shot++;
    }

    return 0;
}

void buildHexGrid(){
    for(int i=0;i<(2*P+1);i++) hexGrid.push_back(vector<Hexagon>());
    for(auto& row : hexGrid) for(int i=0;i<(2*P+1);i++) row.push_back(Hexagon());

    for(int row=0; row<(2*P+1); row++) for(int column=0; column<(2*P+1); column++){ // adjuk meg minden hexagonnak az Q-R koordinatajat
        CoordXY c(column,row);
        hexGrid[row][column].coord=XY2QR(c);
    }

    // invalidaljuk a 2D-s vektorban azokat a hexagonokat, amik nem reszei igazabol a palyanak
    // kerdes: miert nem csak a valid hexagonokat push-oljak a row-kba?
    // valasz: igy letezik XY es QR kozott transzformacio, tehat egy hexagonnak a QR koordinatai alapjan ra tudunk indexelni a szomszedjara
    // egy koztes QR2XY transzformacioval konstans idoben
    for(int row=0; row<P; row++) for(int column=0; column<(P-row);column++) { hexGrid[row][column].isValid=false; hexGrid[row][column].isOut=true; }
    for(int row=P; row>=1; row--) for(int column=2*P; column>(2*P-row);column--) { hexGrid[P+row][column].isValid=false; hexGrid[P+row][column].isOut=true; }
}

void placeHouses(){
    int Qi, Ri, Vi;
    CoordXY coord;
    Hexagon* house;

    for(int i=0;i<N;i++){
        cin >> Qi >> Ri >> Vi;
        coord=QR2XY(CoordQR(Qi,Ri));
        house=&(hexGrid[coord.Y][coord.X]);
        house->roadCount=Vi;
        house->isOut=true;
        if(Vi==0) zeroHouses.push_back(house);
        else houses.push_back(house);
    }
}

Hexagon* getNeighbor(CoordQR coordQR, Direction dir){
    switch(dir){
        case Direction::NORTHEAST:
            coordQR.Q++;
            coordQR.R--;
        break;
        case Direction::EAST:
            coordQR.Q++;
        break;
        case Direction::SOUTHEAST:
            coordQR.R++;
        break;
        case Direction::SOUTHWEST:
            coordQR.Q--;
            coordQR.R++;
        break;
        case Direction::WEST:
            coordQR.Q--;
        break;
        case Direction::NORTHWEST:
            coordQR.R--;
        break;
    }
    CoordXY coordXY=QR2XY(coordQR);
    try{
        if(hexGrid.at(coordXY.Y).at(coordXY.X).isValid) return &(hexGrid.at(coordXY.Y).at(coordXY.X));
        return nullptr;
    }
    catch(out_of_range e){ // ha tulindexelnenk, adjunk vissza nullptr-t
        return nullptr;
    }
}

vector<Direction> fromSOUTHWEST{Direction::SOUTHEAST, Direction::SOUTHWEST, Direction::WEST};
vector<Direction> fromWEST{Direction::SOUTHWEST, Direction::WEST, Direction::NORTHWEST};
vector<Direction> fromNORTHWEST{Direction::WEST, Direction::NORTHWEST, Direction::NORTHEAST};
vector<Direction> fromNORTHEAST{Direction::NORTHWEST, Direction::NORTHEAST, Direction::EAST};
vector<Direction> fromEAST{Direction::NORTHEAST, Direction::EAST, Direction::SOUTHEAST};
vector<Direction> fromSOUTHEAST{Direction::EAST, Direction::SOUTHEAST, Direction::SOUTHWEST};
vector<Direction> whereToGo(Direction from){ // a from itt nem az az irany, ahonnan az erkezesi mezore erkezunk, hanem amerre az elozo mezorol elleptunk
    switch(from){
        case Direction::NORTHEAST:
            return fromNORTHEAST;
        case Direction::EAST:
            return fromEAST;
        case Direction::SOUTHEAST:
            return fromSOUTHEAST;
        case Direction::SOUTHWEST:
            return fromSOUTHWEST;
        case Direction::WEST:
            return fromWEST;
        case Direction::NORTHWEST:
            return fromNORTHWEST;
        default:
            return vector<Direction>{};
    }
}

void crossOutHexagons(){
    checkCorners();
    checkOuterLayer();
    checkInnerLayer();
    checkCloseOnes();
    checkZeros(); // a vegere kell tenni, kulonben elronthatja a vizsgalatokat, ha ervenytelenit mezoket
}

void checkCorners(){
    vector<CoordQR> corners{CoordQR(P,-P),CoordQR(P,0),CoordQR(0,P),CoordQR(-P,P),CoordQR(-P,0),CoordQR(0,-P)};
    vector<vector<Direction>> lookTowards{
        vector<Direction>{Direction::WEST, Direction::SOUTHEAST, Direction::SOUTHWEST},
        vector<Direction>{Direction::NORTHWEST, Direction::SOUTHWEST, Direction::WEST},
        vector<Direction>{Direction::NORTHEAST, Direction::WEST, Direction::NORTHWEST},
        vector<Direction>{Direction::EAST, Direction::NORTHWEST, Direction::NORTHEAST},
        vector<Direction>{Direction::SOUTHEAST, Direction::NORTHEAST, Direction::EAST},
        vector<Direction>{Direction::SOUTHWEST, Direction::EAST, Direction::SOUTHEAST}
    };

    CoordXY cornerXY;
    CoordQR cornerQR;
    Hexagon* neighbor1;
    Hexagon* neighbor2;
    Hexagon* neighbor3; // mindig a neighbor 3 a kozepso szomszed
    for(size_t i=0;i<6; i++){
        cornerQR=corners[i];
        cornerXY=QR2XY(cornerQR);
        neighbor1=getNeighbor(cornerQR,lookTowards[i][0]);
        neighbor2=getNeighbor(cornerQR,lookTowards[i][1]);
        neighbor3=getNeighbor(cornerQR,lookTowards[i][2]);

        if(hexGrid[cornerXY.Y][cornerXY.X].roadCount==1){
            if(neighbor1!=nullptr) neighbor1->isOut=true;
            if(neighbor2!=nullptr) neighbor2->isOut=true;
        }
        else if(neighbor3!=nullptr && (neighbor3->roadCount==1 || neighbor3->roadCount==2)){
            hexGrid[cornerXY.Y][cornerXY.X].isOut=true;
            if(neighbor1!=nullptr) neighbor1->isOut=true;
            if(neighbor2!=nullptr) neighbor2->isOut=true;
        }
        else if(neighbor1!=nullptr && neighbor1->roadCount>0) hexGrid[cornerXY.Y][cornerXY.X].isOut=true;
        else if(neighbor2!=nullptr && neighbor2->roadCount>0) hexGrid[cornerXY.Y][cornerXY.X].isOut=true;
    }
}

void checkOuterLayer(){
    vector<CoordQR> start{CoordQR(P-1,-P),CoordQR(P,-1),CoordQR(1,P-1),CoordQR(-P+1,P),CoordQR(-P,1),CoordQR(-1,-P+1)};
    vector<vector<Direction>> crossOut{ // a ket szomszed, amit ki kell huzni, mellesleg az elobbi dir iranyaba kell lepkedni
        vector<Direction>{Direction::WEST, Direction::EAST},
        vector<Direction>{Direction::NORTHWEST, Direction::SOUTHEAST},
        vector<Direction>{Direction::NORTHEAST, Direction::SOUTHWEST},
        vector<Direction>{Direction::EAST, Direction::WEST},
        vector<Direction>{Direction::SOUTHEAST, Direction::NORTHWEST},
        vector<Direction>{Direction::SOUTHWEST, Direction::NORTHEAST}
    };

    CoordXY startXY;
    CoordQR startQR;
    Hexagon* neighbor1;
    Hexagon* neighbor2;

    for(size_t i=0;i<6;i++){
        startQR=start[i];
        startXY=QR2XY(startQR);
        for(int j=0;j<P-1;j++){ // P-1 mezot vizsgalunk egy oldalon, tehat P-2-szor lepunk
            if(hexGrid[startXY.Y][startXY.X].roadCount==1){
                neighbor1=getNeighbor(startQR,crossOut[i][0]);
                neighbor2=getNeighbor(startQR,crossOut[i][1]);
                if(neighbor1!=nullptr) neighbor1->isOut=true;
                if(neighbor2!=nullptr) neighbor2->isOut=true;
            }
            startQR=(getNeighbor(startQR,crossOut[i][0]))->coord; // ezek a szomszedok biztosan leteznek
            startXY=QR2XY(startQR);
        }
    }
}

void checkInnerLayer(){
    vector<CoordQR> start{CoordQR(P-2,-P+1),CoordQR(P-1,-1),CoordQR(1,P-2),CoordQR(-P+2,P-1),CoordQR(-P+1,1),CoordQR(-1,-P+2)};
    vector<vector<Direction>> MoveAndCrossOut{ // az elso a haladasi irany, a masik ketto a ket szomszed, amit ki kell huzni
        vector<Direction>{Direction::WEST, Direction::NORTHWEST, Direction::NORTHEAST},
        vector<Direction>{Direction::NORTHWEST, Direction::NORTHEAST, Direction::EAST},
        vector<Direction>{Direction::NORTHEAST, Direction::EAST, Direction::SOUTHEAST},
        vector<Direction>{Direction::EAST, Direction::SOUTHEAST, Direction::SOUTHWEST},
        vector<Direction>{Direction::SOUTHEAST, Direction::WEST, Direction::SOUTHWEST},
        vector<Direction>{Direction::SOUTHWEST, Direction::WEST, Direction::NORTHWEST}
    };

    CoordXY startXY;
    CoordQR startQR;
    Hexagon* neighbor1;
    Hexagon* neighbor2;

    for(size_t i=0;i<6;i++){
        startQR=start[i];
        startXY=QR2XY(startQR);
        for(int j=0;j<P-2;j++){ // P-2 mezot vizsgalunk egy oldalon, tehat P-3-szor lepunk
            if(hexGrid[startXY.Y][startXY.X].roadCount==1){
                neighbor1=getNeighbor(startQR,MoveAndCrossOut[i][1]);
                neighbor2=getNeighbor(startQR,MoveAndCrossOut[i][2]);
                if(neighbor1!=nullptr) neighbor1->isOut=true;
                if(neighbor2!=nullptr) neighbor2->isOut=true;
            }
            startQR=(getNeighbor(startQR,MoveAndCrossOut[i][0]))->coord; // ezek a szomszedok biztosan leteznek
            startXY=QR2XY(startQR);
        }
    }
}

void checkCloseOnes(){
    vector<vector<Direction>> crossOut{ // a ket szomszed, amit ki kell huzni
        vector<Direction>{Direction::NORTHWEST, Direction::EAST},
        vector<Direction>{Direction::NORTHEAST, Direction::SOUTHEAST},
        vector<Direction>{Direction::EAST, Direction::SOUTHWEST},
        vector<Direction>{Direction::SOUTHEAST, Direction::WEST},
        vector<Direction>{Direction::SOUTHWEST, Direction::NORTHWEST},
        vector<Direction>{Direction::WEST, Direction::NORTHEAST}
    };

    vector<vector<Direction>> lookAndCross{ // elso irany: a tavolabbi szomszedtol hova nezzunk; masodik irany: ha 1-est talaltunk, mit huzzunk ki
        vector<Direction>{Direction::SOUTHEAST, Direction::EAST},
        vector<Direction>{Direction::SOUTHWEST, Direction::SOUTHEAST},
        vector<Direction>{Direction::WEST, Direction::SOUTHWEST},
        vector<Direction>{Direction::NORTHWEST, Direction::WEST},
        vector<Direction>{Direction::NORTHEAST, Direction::NORTHWEST},
        vector<Direction>{Direction::EAST, Direction::NORTHEAST}
    };

    Hexagon* neighbor;
    Hexagon* fartherNeighbor1;
    Hexagon* fartherNeighbor2;
    Hexagon* crossNeighbor1;
    Hexagon* crossNeighbor2;
    for(auto house : houses){
        if(house->roadCount==1){
            for(size_t i=0; i<6; i++){ // NORTHEAST, EAST, etc.
                neighbor=getNeighbor(house->coord,directions[i]);
                if(neighbor!=nullptr){
                    if(neighbor->roadCount==1){
                        crossNeighbor1=getNeighbor(house->coord,crossOut[i][0]);
                        crossNeighbor2=getNeighbor(house->coord,crossOut[i][1]);
                        if(crossNeighbor1!=nullptr) crossNeighbor1->isOut=true;
                        if(crossNeighbor2!=nullptr) crossNeighbor2->isOut=true;
                    }
                    fartherNeighbor1=getNeighbor(neighbor->coord,directions[i]);
                    if(fartherNeighbor1!=nullptr){
                        if(fartherNeighbor1->roadCount==1) neighbor->isOut=true;
                        fartherNeighbor2=getNeighbor(fartherNeighbor1->coord,lookAndCross[i][0]);
                        if(fartherNeighbor2!=nullptr && fartherNeighbor2->roadCount==1){
                            neighbor->isOut=true;
                            crossNeighbor1=getNeighbor(house->coord,lookAndCross[i][1]);
                            if(crossNeighbor1!=nullptr) crossNeighbor1->isOut=true;
                        }
                    }
                }
            }
        }
    }
}

void checkZeros(){
    Hexagon* neighbor;
    for(Hexagon* zeroHouse : zeroHouses){
        for (auto dir : directions){
            neighbor=getNeighbor(zeroHouse->coord,dir);
            if(neighbor!=nullptr && neighbor->roadCount<1) neighbor->isValid=false;
        }
        zeroHouse->isValid=false;
    }
}

void collectHouseNeighbors(){
    Hexagon* neighbor;
    for(size_t i=0;i<houses.size();i++) houseNeighbors.push_back(vector<Hexagon*>());

    for(size_t i=0;i<houses.size();i++){
        houseRoadCounts.push_back(houses[i]->roadCount);
        for(auto dir : directions){
            neighbor=getNeighbor(houses[i]->coord,dir);
            if(neighbor!=nullptr && neighbor->isOut==false) houseNeighbors[i].push_back(neighbor);
        }
    }
}

void findRoadPiece(){
    int counter;
    Hexagon* neighbor;
    bool foundOne=false;
    for(size_t j=0; j<houses.size() && !foundOne; j++){
        counter=6;
        for(auto dir : directions){
            neighbor=getNeighbor(houses[j]->coord, dir);
            if(neighbor==nullptr || neighbor->isOut) counter--;
        }
        if(counter==houses[j]->roadCount) for(auto dir : directions){
            neighbor=getNeighbor(houses[j]->coord, dir);
            if(neighbor!=nullptr && neighbor->isOut==false){
                    neighbor->mustBeRoad=true;
                    start=neighbor;
                    foundOne=true;
                    break;
            }
        }
    }
}

void fillStartNeighbors(){
    Hexagon* neighbor;
    for(auto dir : directions){
        neighbor=getNeighbor(start->coord,dir);
        if(neighbor!=nullptr && neighbor->isOut==false){
                startNeighbors.push_back(neighbor);
                startDirections.push_back(dir);
        }
    }
}
