#include "soko_game.h"
#include "soko.h"
#include "soko_gamerules.h"

#include "shapes.h"

//True if the entity CANNOT go on the tile
bool sokoEntityTileCollision[5][8] = {
    //Empty, //floor  //wall   //goal    //portal  //l-emit  //l-receive //walked
    {true,    false,    true,    false,    false,    false,    false,      false},//SKE_NONE
    {true,    false,    true,    false,    false,    false,    false,      true},//PLAYER
    {true,    false,    true,    false,    false,    false,    false,      false},//CRATE
    {true,    false,    true,    false,    false,    false,    false,      false},//LASER
    {true,    false,    true,    false,    false,    false,    false,      false},//STICKY CRATE
};

uint64_t victoryDanceTimer;

void sokoConfigGamemode(soko_abs_t* gamestate, soko_var_t variant) //This should be called when you reload a level to make sure game rules are correct
{
    if(variant == SOKO_CLASSIC) //standard gamemode. Check 'variant' variable
    {
        printf("Config Soko to Classic\n");
        gamestate->gameLoopFunc = absSokoGameLoop;
        gamestate->sokoTryPlayerMovementFunc = absSokoTryPlayerMovement;
        gamestate->sokoTryMoveEntityInDirectionFunc = absSokoTryMoveEntityInDirection;
        gamestate->drawTilesFunc = absSokoDrawTiles;
        gamestate->isVictoryConditionFunc = absSokoAllCratesOnGoal;
        gamestate->sokoGetTileFunc = absSokoGetTile;
    }else if(variant == SOKO_EULER) //standard gamemode. Check 'variant' variable
    {
        printf("Config Soko to Euler\n");
        gamestate->gameLoopFunc = absSokoGameLoop;
        gamestate->sokoTryPlayerMovementFunc = eulerSokoTryPlayerMovement;
        gamestate->sokoTryMoveEntityInDirectionFunc = absSokoTryMoveEntityInDirection;
        gamestate->drawTilesFunc = absSokoDrawTiles;
        gamestate->isVictoryConditionFunc = eulerNoUnwalkedFloors;
        gamestate->sokoGetTileFunc = absSokoGetTile;
    }
    else if(variant == SOKO_OVERWORLD)
    {
        printf("Config Soko to Overworld\n");
        gamestate->gameLoopFunc = overworldSokoGameLoop;
        gamestate->sokoTryPlayerMovementFunc = absSokoTryPlayerMovement;
        gamestate->sokoTryMoveEntityInDirectionFunc = absSokoTryMoveEntityInDirection;
        gamestate->drawTilesFunc = absSokoDrawTiles;
        gamestate->isVictoryConditionFunc = overworldPortalEntered;
        gamestate->sokoGetTileFunc = absSokoGetTile;
    }
    else if (variant == SOKO_LASERBOUNCE)
    {
        printf("Config Soko to Laser Bounce\n");
        gamestate->gameLoopFunc = laserBounceSokoGameLoop;
        gamestate->sokoTryPlayerMovementFunc = absSokoTryPlayerMovement;
        gamestate->sokoTryMoveEntityInDirectionFunc = absSokoTryMoveEntityInDirection;
        gamestate->drawTilesFunc = absSokoDrawTiles;
        gamestate->isVictoryConditionFunc = absSokoAllCratesOnGoal;
        gamestate->sokoGetTileFunc = absSokoGetTile;
    }
    
    //add conditional for alternative variants
}



void laserBounceSokoGameLoop(soko_abs_t *self, int64_t elapsedUs)
{
    if(self->state == SKS_GAMEPLAY)
    {
        //logic
        self->sokoTryPlayerMovementFunc(self);

        //victory status. stored separate from gamestate because of future gameplay ideas/remixes.
        //todo: rename to isVictory or such.
        self->allCratesOnGoal = self->isVictoryConditionFunc(self);
        if(self->allCratesOnGoal){
            self->state = SKS_VICTORY;
            victoryDanceTimer = 0;
        }
        //draw level
        self->drawTilesFunc(self, &self->currentLevel);
        drawLaserFromEntity(self,self->soko_player);
    }else if(self->state == SKS_VICTORY)
    {
        //check for input for exit/next level.
        self->drawTilesFunc(self, &self->currentLevel);
        victoryDanceTimer += elapsedUs;
        if(victoryDanceTimer > SOKO_VICTORY_TIMER_US)
        {
            self->loadNewLevelIndex = 0;
            self->loadNewLevelFlag = true;
            self->screen = SOKO_LOADNEWLEVEL;
        }
    }


    //DEBUG PLACEHOLDER:
            // Render the time to a string
            char str[16] = {0};
            int16_t tWidth;
            if(!self->allCratesOnGoal)
            {
                snprintf(str, sizeof(str) - 1, "sokoban");
                // Measure the width of the time string
                tWidth = textWidth(&self->ibm, str);
                // Draw the time string to the display, centered at (TFT_WIDTH / 2)
                drawText(&self->ibm, c555, str, ((TFT_WIDTH - tWidth) / 2), 0);
            }else
            {
                snprintf(str, sizeof(str) - 1, "sokasuccess");
                // Measure the width of the time string
                tWidth = textWidth(&self->ibm, str);
                // Draw the time string to the display, centered at (TFT_WIDTH / 2)
                drawText(&self->ibm, c555, str, ((TFT_WIDTH - tWidth) / 2), 0);
            }
            sharedGameLoop(self);

}


void absSokoGameLoop(soko_abs_t *self, int64_t elapsedUs)
{
    if(self->state == SKS_GAMEPLAY)
    {
        //logic
        self->sokoTryPlayerMovementFunc(self);

        //victory status. stored separate from gamestate because of future gameplay ideas/remixes.
        //todo: rename to isVictory or such.
        self->allCratesOnGoal = self->isVictoryConditionFunc(self);
        if(self->allCratesOnGoal){
            self->state = SKS_VICTORY;
            victoryDanceTimer = 0;
        }
        //draw level
        self->drawTilesFunc(self, &self->currentLevel);

    }else if(self->state == SKS_VICTORY)
    {
        //check for input for exit/next level.
        self->drawTilesFunc(self, &self->currentLevel);
        victoryDanceTimer += elapsedUs;
        if(victoryDanceTimer > SOKO_VICTORY_TIMER_US)
        {
            self->loadNewLevelIndex = 0;
            self->loadNewLevelFlag = true;
            self->screen = SOKO_LOADNEWLEVEL;
        }
    }


    //DEBUG PLACEHOLDER:
            char str[16] = {0};
            int16_t tWidth;
            if(!self->allCratesOnGoal)
            {
                snprintf(str, sizeof(str) - 1, "sokoban");
                // Measure the width of the time string
                tWidth = textWidth(&self->ibm, str);
                // Draw the time string to the display, centered at (TFT_WIDTH / 2)
                drawText(&self->ibm, c555, str, ((TFT_WIDTH - tWidth) / 2), 0);
            }else
            {
                snprintf(str, sizeof(str) - 1, "sokasuccess");
                // Measure the width of the time string
                tWidth = textWidth(&self->ibm, str);
                // Draw the time string to the display, centered at (TFT_WIDTH / 2)
                drawText(&self->ibm, c555, str, ((TFT_WIDTH - tWidth) / 2), 0);
            }
            sharedGameLoop(self);
}

void sharedGameLoop(soko_abs_t *self){
    if(self->input.restartLevel)
    {
        restartCurrentLevel(self);
    }else if(self->input.exitToOverworld)
    {
        exitToOverworld(self);
    }
}

//Gameplay Logic
void absSokoTryPlayerMovement(soko_abs_t *self)
{
    
    if(self->input.playerInputDeltaX == 0 && self->input.playerInputDeltaY == 0)
    {
        return;
    }

    self->sokoTryMoveEntityInDirectionFunc(self, self->soko_player,self->input.playerInputDeltaX,self->input.playerInputDeltaY,0);
}


bool absSokoTryMoveEntityInDirection(soko_abs_t *self, sokoEntity_t* entity, int dx, int dy, uint16_t push)
{
    //prevent infitnite loop where you push yourself nowhere.
    if(dx == 0 && dy == 0 )
    {
        return false;
    }

    //maxiumum number of crates we can push. Traditional sokoban has a limit of one. I prefer infinite for challenges.
    if(self->maxPush != 0 && push>self->maxPush)
    {
        return false;
    }

    int px = entity->x+dx;
    int py = entity->y+dy;
    sokoTile_t nextTile = self->sokoGetTileFunc(self,px,py);

    //when this is false, we CAN move. True for Collision.
    if(!sokoEntityTileCollision[entity->type][nextTile])
    {
        //Is there an entity at this position?
        for (size_t i = 0; i < self->currentLevel.entityCount; i++)
        {
            //is pushable.
            if(self->currentLevel.entities[i].type == SKE_CRATE || self->currentLevel.entities[i].type == SKE_STICKY_CRATE)
            {
                if(self->currentLevel.entities[i].x == px && self->currentLevel.entities[i].y == py)
                {
                    if(self->sokoTryMoveEntityInDirectionFunc(self,&self->currentLevel.entities[i],dx,dy,push+1))
                    {
                        printf("pushing entity");
                        entity->x += dx;
                        entity->y += dy;
                        entity->facing = sokoDirectionFromDelta(dx,dy);
                        return true;
                    }else{
                        //can't push? can't move.
                        return false;
                    }
                   
                }
            }
            
        }
        
        //No wall in front of us and nothing to push, we can move.
        entity->x += dx;
        entity->y += dy;
        entity->facing = sokoDirectionFromDelta(dx,dy);
        return true;
    }//all other floor types invalid. Be careful when we add tile types in different rule sets.
    
    return false;
}

//draw the tiles (and entities, for now) of the level.
void absSokoDrawTiles(soko_abs_t* self, sokoLevel_t* level)
{
    SETUP_FOR_TURBO();
    uint16_t scale = level->levelScale;
    uint16_t ox = (TFT_WIDTH/2)-((level->width)*scale/2);
    uint16_t oy = (TFT_HEIGHT/2)-((level->height)*scale/2);

    for (size_t x = 0; x < level->width; x++)
    {
        for (size_t y = 0; y < level->height; y++)
        {
            paletteColor_t color = cTransparent;
            switch (level->tiles[x][y])
            {
                case SKT_FLOOR:
                    color = c444;
                    break;
                case SKT_WALL:
                    color = c111;
                    break;
                case SKT_GOAL:
                    color = c141;
                    break;
                case SKT_FLOOR_WALKED:
                    color = c334;
                    break;
                case SKT_EMPTY:
                    color = cTransparent;
                    break;
                case SKT_PORTAL:
                    color = c440;
                    break;
                default:
                    break;
            }
        
            //Draw a square.
            //none of this matters it's all getting replaced with drawwsg later.
            if(color != cTransparent){
                for (size_t xd = ox+x*scale; xd < ox+x*scale+scale; xd++)
                {
                    for (size_t yd = oy+y*scale; yd < oy+y*scale+scale; yd++)
                    {
                        TURBO_SET_PIXEL(xd, yd, color);
                    }
                }
            }
            //draw outline around the square.
            //drawRect(ox+x*s,oy+y*s,ox+x*s+s,oy+y*s+s,color);
        }
    }

    for (size_t i = 0; i < level->entityCount; i++)
    {
        switch (level->entities[i].type)
        {
            case SKE_PLAYER:
                switch(level->entities[i].facing){
                    case SKD_UP:
                        drawWsg(&self->playerUpWSG,ox+level->entities[i].x*scale,oy+level->entities[i].y*scale,false,false,0);
                        break;
                    case SKD_RIGHT:
                        drawWsg(&self->playerRightWSG,ox+level->entities[i].x*scale,oy+level->entities[i].y*scale,false,false,0);
                        break;
                    case SKD_LEFT:
                        drawWsg(&self->playerLeftWSG,ox+level->entities[i].x*scale,oy+level->entities[i].y*scale,false,false,0);
                        break;
                    case SKD_DOWN:
                    default:
                        drawWsg(&self->playerDownWSG,ox+level->entities[i].x*scale,oy+level->entities[i].y*scale,false,false,0);
                        break;
                }
                
                break;
            case SKE_CRATE:
                 drawWsg(&self->crateWSG,ox+level->entities[i].x*scale,oy+level->entities[i].y*scale,false,false,0);
                 break;
            case SKE_STICKY_CRATE:
                 drawWsg(&self->stickyCrateWSG,ox+level->entities[i].x*scale,oy+level->entities[i].y*scale,false,false,0);
                break;
            case SKE_NONE:
            default:
                break;
        }
    }
    
}

/*
//draw the tiles (and entities, for now) of the level.
void absSokoDrawTiles(soko_abs_t *self, sokoLevel_t* level)
{
    SETUP_FOR_TURBO();
    uint16_t scale = level->levelScale;
    uint16_t ox = (TFT_WIDTH/2)-((level->width)*scale/2);
    uint16_t oy = (TFT_HEIGHT/2)-((level->height)*scale/2);

    for (size_t x = 0; x < level->width; x++)
    {
        for (size_t y = 0; y < level->height; y++)
        {
            paletteColor_t color = cTransparent;
            switch (level->tiles[x][y])
            {
                case SKT_FLOOR:
                    color = c444;
                    break;
                case SKT_WALL:
                    color = c111;
                    break;
                case SKT_GOAL:
                    color = c141;
                    break;
                case SKT_EMPTY:
                    color = cTransparent;
                default:
                    break;
            }
        
            //Draw a square.
            //none of this matters it's all getting replaced with drawwsg later.
            if(color != cTransparent){
                for (size_t xd = ox+x*scale; xd < ox+x*scale+scale; xd++)
                {
                    for (size_t yd = oy+y*scale; yd < oy+y*scale+scale; yd++)
                    {
                        TURBO_SET_PIXEL(xd, yd, color);
                    }
                }
            }
            //draw outline around the square.
            //drawRect(ox+x*s,oy+y*s,ox+x*s+s,oy+y*s+s,color);
        }
    }

    for (size_t i = 0; i < level->entityCount; i++)
    {
        switch (level->entities[i].type)
        {
            case SKE_PLAYER:
                // drawCircleFilled(ox+level->entities[i].x*scale+scale/2,oy+level->entities[i].y*scale+scale/2,scale/2-1,c411);
                drawWsg(&self->playerWSG,ox+level->entities[i].x*scale,oy+level->entities[i].y*scale,false,false,0);
                break;
            case SKE_CRATE:
                //drawCircleFilled(ox+level->entities[i].x*scale+scale/2,oy+level->entities[i].y*scale+scale/2,scale/2-1,c441);
                 drawWsg(&self->crateWSG,ox+level->entities[i].x*scale,oy+level->entities[i].y*scale,false,false,0);
            case SKE_NONE:
            default:
                break;
        }
    }
    
}
*/
bool absSokoAllCratesOnGoal(soko_abs_t *self)
{
    for (size_t i = 0; i < self->currentLevel.entityCount; i++)
    {
        if(self->currentLevel.entities[i].type == SKE_CRATE)
        {
            if(self->currentLevel.tiles[self->currentLevel.entities[i].x][self->currentLevel.entities[i].y] != SKT_GOAL)
            {
                return false;
            }
        }
    }

    return true;
}

sokoTile_t absSokoGetTile(soko_abs_t *self, int x, int y)
{
    if(x<0 || x >= self->currentLevel.width)
    {
        return SKT_WALL;
    }
    if(y<0 || y >= self->currentLevel.height)
    {
        return SKT_WALL;
    }
     
    return self->currentLevel.tiles[x][y];
}

sokoDirection_t sokoDirectionFromDelta(int dx,int dy)
{
    if(dx > 0 && dy == 0)
    {
        return SKD_RIGHT;
    }else  if(dx < 0 && dy == 0)
    {
        return SKD_LEFT;
    }else  if(dx == 0 && dy < 0)
    {
        return SKD_UP;
    }else  if(dx == 0 && dy > 0)
    {
        return SKD_DOWN;
    }

    return SKD_NONE;
}

sokoVec_t sokoGridToPix(soko_abs_t* self,sokoVec_t grid) //Convert grid position to screen pixel position
{
    sokoVec_t retVec;
    SETUP_FOR_TURBO();
    uint16_t scale = self->currentLevel.levelScale; //@todo These should be in constants, but too lazy to change all references at the moment.
    uint16_t ox = (TFT_WIDTH/2)-((self->currentLevel.width)*scale/2);
    uint16_t oy = (TFT_HEIGHT/2)-((self->currentLevel.height)*scale/2); 
    retVec.x = ox+scale*grid.x+scale/2;
    retVec.y = oy+scale*grid.y+scale/2;
    return retVec;
}

void drawLaserFromEntity(soko_abs_t* self, sokoEntity_t* emitter)
{
    sokoCollision_t impactSpot = sokoBeamImpact(self, self->soko_player);
    //printf("Player Pos: x:%d,y:%d Facing:%d Impact Result: x:%d,y:%d, Flag:%d Index:%d\n",self->soko_player->x,self->soko_player->y,self->soko_player->facing,impactSpot.x,impactSpot.y,impactSpot.entityFlag,impactSpot.entityIndex);
    sokoVec_t playerGrid,impactGrid;
    playerGrid.x = emitter->x;
    playerGrid.y = emitter->y;
    impactGrid.x = impactSpot.x;
    impactGrid.y = impactSpot.y;
    sokoVec_t playerPix = sokoGridToPix(self,playerGrid);
    sokoVec_t impactPix = sokoGridToPix(self,impactGrid);
    drawLine(playerPix.x,playerPix.y,impactPix.x,impactPix.y,c500,0);

}

sokoCollision_t sokoBeamImpact(soko_abs_t* self, sokoEntity_t* emitter)
{
    sokoDirection_t dir = emitter->facing;
    sokoVec_t projVec = {0, 0};
    sokoVec_t emitVec = {emitter->x,emitter->y};
    switch (dir){
    case SKD_DOWN:
        projVec.y = 1;
        break;
    case SKD_UP:
        projVec.y = -1;
        break;
    case SKD_LEFT:
        projVec.x = -1;
        break;
    case SKD_RIGHT:
        projVec.x = 1;
        break;
    default:
        //return base entity position
    }
    
    
    //Iterate over tiles in ray to edge of level
    sokoVec_t testPos = sokoAddCoord(emitVec,projVec);
    int entityCount = self->currentLevel.entityCount;
    //todo: make first pass pack a statically allocated array with only the entities in the path of the laser.
    
    uint8_t tileCollision[] = {0,0,1,0,0,1,1}; //There should be a pointer internal to the game state so this can vary with game mode
    uint8_t entityCollision[] = {0,0,1,1};

    int16_t possibleSquares = 0;
    if(dir==SKD_RIGHT) //move these checks into the switch statement
    {
        possibleSquares = self->currentLevel.width - emitVec.x; //Up to and including far wall
    }
    if(dir==SKD_LEFT)
    {
        possibleSquares = emitVec.x + 1;
    }
    if(dir==SKD_UP)
    {
        possibleSquares = emitVec.y + 1;
    }
    if(dir==SKD_DOWN)
    {
        possibleSquares = self->currentLevel.height - emitVec.y;
    }

    int tileCollFlag, entCollFlag, entCollInd;
    tileCollFlag = entCollFlag = entCollInd = 0;

    sokoCollision_t retVal;
    //printf("emitVec(%d,%d)",emitVec.x,emitVec.y);
    //printf("projVec:(%d,%d) possibleSquares:%d ",projVec.x,projVec.y,possibleSquares);

    for (int n=0;n < possibleSquares; n++)
    {
        sokoTile_t posTile = absSokoGetTile(self,testPos.x,testPos.y);
        //printf("|n:%d,posTile:(%d,%d):%d|",n,testPos.x,testPos.y,posTile);
        if(tileCollision[posTile])
        {
            tileCollFlag = 1;
            break;
        }
        for(int m = 0;m < entityCount;m++) //iterate over tiles/entities to check for laser collision. First pass finds everything in the path of the 
        {
            sokoEntity_t candidateEntity = self->currentLevel.entities[m];
            //printf("|m:%d;CE:(%d,%d)%d",m,candidateEntity.x,candidateEntity.y,candidateEntity.type);
            if(candidateEntity.x == testPos.x && candidateEntity.y == testPos.y)
            {
                //printf(";POSMATCH;Coll:%d",entityCollision[candidateEntity.type]);
                if(entityCollision[candidateEntity.type])
                {
                    entCollFlag = 1;
                    entCollInd = m;
                    //printf("|");
                    break;
                }

            }
            //printf("|");
        }
        
        if(entCollFlag)
        {
            break;
        }
        testPos = sokoAddCoord(testPos,projVec);
    }
    //printf("\n");
    retVal.x = testPos.x;
    retVal.y = testPos.y;
    retVal.entityIndex = entCollInd;
    retVal.entityFlag = entCollFlag;
    //printf("impactPoint:(%d,%d)\n",testPos.x,testPos.y);
    return retVal;

}

sokoVec_t sokoAddCoord(sokoVec_t op1, sokoVec_t op2)
{
    sokoVec_t retVal;
    retVal.x = op1.x + op2.x;
    retVal.y = op1.y + op2.y;
    return retVal;
}


//Euler Game Modes
void eulerSokoTryPlayerMovement(soko_abs_t *self)
{
    if(self->input.playerInputDeltaX == 0 && self->input.playerInputDeltaY == 0)
    {
        return;
    }

    uint16_t x = self->soko_player->x;
    uint16_t y = self->soko_player->y;
    bool moved = self->sokoTryMoveEntityInDirectionFunc(self, self->soko_player,self->input.playerInputDeltaX,self->input.playerInputDeltaY,0);
    
    if(moved)
    {
        //Paint Floor

        //previous
        if(self->currentLevel.tiles[x][y] == SKT_FLOOR)
        {
            self->currentLevel.tiles[x][y] = SKT_FLOOR_WALKED;
        }
        //current
        if(self->currentLevel.tiles[self->soko_player->x][self->soko_player->y] == SKT_FLOOR)
        {
            self->currentLevel.tiles[self->soko_player->x][self->soko_player->y] = SKT_FLOOR_WALKED;
        }

        //Try Sticky Blocks
        //Loop through all entities is probably not really slower than sampling? We usually have <5 entities.
        for (size_t i = 0; i < self->currentLevel.entityCount; i++)
        {
            if(self->currentLevel.entities[i].type == SKE_STICKY_CRATE)
            {
                if(self->currentLevel.entities[i].x == x && self->currentLevel.entities[i].y == y+1)
                {
                    absSokoTryMoveEntityInDirection(self,&self->currentLevel.entities[i],self->input.playerInputDeltaX,self->input.playerInputDeltaY,0);
                }else if(self->currentLevel.entities[i].x == x && self->currentLevel.entities[i].y == y-1){
                    absSokoTryMoveEntityInDirection(self,&self->currentLevel.entities[i],self->input.playerInputDeltaX,self->input.playerInputDeltaY,0);
                }else if(self->currentLevel.entities[i].y == y && self->currentLevel.entities[i].x == x+1){
                    absSokoTryMoveEntityInDirection(self,&self->currentLevel.entities[i],self->input.playerInputDeltaX,self->input.playerInputDeltaY,0);
                }else if(self->currentLevel.entities[i].y == y && self->currentLevel.entities[i].x == x-1){
                    absSokoTryMoveEntityInDirection(self,&self->currentLevel.entities[i],self->input.playerInputDeltaX,self->input.playerInputDeltaY,0);
                }
            }
        }
    }
}

bool eulerNoUnwalkedFloors(soko_abs_t *self)
{
    for (size_t x = 0; x < self->currentLevel.width; x++)
    {
        for (size_t y = 0; y < self->currentLevel.height; y++)
        {
            if(self->currentLevel.tiles[x][y] == SKT_FLOOR)
            {
                return false;
            }
        }
    }

    return true;
}

void overworldSokoGameLoop(soko_abs_t *self, int64_t elapsedUs)
{
    if(self->state == SKS_GAMEPLAY)
    {
        //logic
        self->sokoTryPlayerMovementFunc(self);

        //victory status. stored separate from gamestate because of future gameplay ideas/remixes.
        //todo: rename to isVictory or such.
        self->allCratesOnGoal = self->isVictoryConditionFunc(self);
        if(self->allCratesOnGoal){
            self->state = SKS_VICTORY;
            printf("Player at %d,%d\n",self->soko_player->x,self->soko_player->y);
            victoryDanceTimer = 0;
        }
        //draw level
        self->drawTilesFunc(self, &self->currentLevel);

    }else if(self->state == SKS_VICTORY)
    {
        
        self->drawTilesFunc(self, &self->currentLevel);
        
        //check for input for exit/next level.
        uint8_t targetWorldIndex = 0;
        for(int i = 0; i < self->portalCount; i++)
        {
            if(self->soko_player->x == self->portals[i].x && self->soko_player->y == self->portals[i].y)
            {
                targetWorldIndex = i+1;
                break;
            }
        }
        printf("Player at %d,%d\n",self->soko_player->x,self->soko_player->y);
        self->loadNewLevelIndex = targetWorldIndex;
        self->loadNewLevelFlag = true;
        self->screen = SOKO_LOADNEWLEVEL;
        
    }


    //DEBUG PLACEHOLDER:
            // Render the time to a string
            char str[16] = {0};
            int16_t tWidth;
            if(!self->allCratesOnGoal)
            {
                snprintf(str, sizeof(str) - 1, "sokoban");
                // Measure the width of the time string
                tWidth = textWidth(&self->ibm, str);
                // Draw the time string to the display, centered at (TFT_WIDTH / 2)
                drawText(&self->ibm, c555, str, ((TFT_WIDTH - tWidth) / 2), 0);
            }else
            {
                snprintf(str, sizeof(str) - 1, "sokasuccess");
                // Measure the width of the time string
                tWidth = textWidth(&self->ibm, str);
                // Draw the time string to the display, centered at (TFT_WIDTH / 2)
                drawText(&self->ibm, c555, str, ((TFT_WIDTH - tWidth) / 2), 0);
            }
}

bool overworldPortalEntered(soko_abs_t *self)
{
    for(uint8_t i = 0; i < self->portalCount; i++)
    {
        if(self->soko_player->x == self->portals[i].x && self->soko_player->y == self->portals[i].y)
        {
            return true;
        }
    }
    return false;
}

void restartCurrentLevel(soko_abs_t *self)
{
    //assumed this is set already?
    //self->loadNewLevelIndex = self->loadNewLevelIndex;

    //todo: what can we do about screen flash when restarting?
    self->loadNewLevelFlag = true;
    self->screen = SOKO_LOADNEWLEVEL;
}

void exitToOverworld(soko_abs_t *self)
{
    self->loadNewLevelIndex = 0;
    self->loadNewLevelFlag = true;
    // self->state = SKS_GAMEPLAY;
    self->screen = SOKO_LOADNEWLEVEL;
}
