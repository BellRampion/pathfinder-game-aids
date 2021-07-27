/*
This program calculates the modifiers, not including ability score bonuses, for attack and damage rolls in standard Pathfinder.

Copyright (C) 2021 Bailie Livingston

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/



#include <stdio.h>
#include "attackDecls.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

void loadFeats(int baseAttackBonus);
int myGetline(char s[], int maxLen, int putNl);
void printMenu(char selections[]);
void charArrayToIntArray(char charArr[], int intArr[], char separator, int acceptNewline);
int copySubStr(char dest[], char src[], int srcStart, int srcEnd, int maxLen);

int main(){
    int bab = 0;
    //BAB shouldn't take up much space.
    char baseAttackBonus[BABLEN];
    char tempChar = 0;
    int buildType;
    char charChosen[MAXLENGTH];
    int intChosen[MAXLENGTH];
    int finalDamage[MAXATTACKS];
    int finalAttack[MAXATTACKS];
    char tempArr[MAXLENGTH];
    int attackAbilityMod;
    int damageAbilityMod;
    int finalRangePenalty = RANGE_DEFAULT_PENALTY;
    //Flags
    short extraAttack = FALSE;
    short numAttacks = 0;
    short baseNumAttacks = 0;

    //License notice
    printf("calcAttackMods.c Copyright (C) 2021 Bailie Livingston\nThis program comes with ABSOLUTELY NO WARRANTY; for details see attached license (\"GPL License.txt\"). \n This is free software, and you are welcome to redistribute it under certain conditions; see attached license for details (\"GPL License.txt\")\n\n");

    //Get base attack bonus
    printf("Please enter your HIGHEST base attack bonus (e.g. +16): ");
    myGetline(baseAttackBonus, BABLEN, FALSE);
    bab = atoi(baseAttackBonus);
    //Fill baseAttackBonus array - highest to lowest
    for (int i = 0; i < BABLEN; i++){
        baseAttackBonus[i] = bab - (i * 5);
        if (baseAttackBonus[i] <= 0){
            baseAttackBonus[i] = 0;
            if (baseNumAttacks == 0){
                baseNumAttacks = i;
            }
        }
    }
    numAttacks = baseNumAttacks;

    //Determine if build is ranged or melee
    printf("Show options for a ranged build or a melee build? (%i or %i): ", RANGED, MELEE);
    tempChar = getchar();
    //Gets the newline
    tempChar = getchar();
    if (tempChar == '1'){
        buildType = MELEE;
    }
    else {
        buildType = RANGED;
    }

    //Get the ability mod for the attack and the one for the damage
    printf("Please enter the ability score modifer for this attack: ");
    myGetline(tempArr, MAXLENGTH, FALSE);
    attackAbilityMod = atoi(tempArr);
    printf("Please enter the ability score modifier for the damage (if none, enter 0): ");
    myGetline(tempArr, MAXLENGTH, FALSE);
    damageAbilityMod = atoi(tempArr);

    //Now calculate data for the feats
    loadFeats(bab);
    //Initialize final attack and damage arrays
    for (int i = 0; i < MAXATTACKS; i++){
        finalAttack[i] = 0;
        finalDamage[i] = 0;
    }

    if (buildType == MELEE){
        printf("Melee is not supported yet. Ask the developer to add it.\n");
        return 0;
    }
    else {
        printMenu(charChosen);
        charArrayToIntArray(charChosen, intChosen, ' ', TRUE);
    }

    for (int i = 0; intChosen[i] != '\0'; i++){
        //Convert the number to an array index (currently one higher than the index)
        intChosen[i]--;

        //Apply the correct bonuses and penalties for each feat chosen
        if (featList[intChosen[i]].applyToAll == TRUE){
            for (int iter = 0; iter < MAXATTACKS; iter++){
                finalAttack[iter] += featList[intChosen[i]].attackMod;
                finalDamage[iter] += featList[intChosen[i]].damageMod;
            }
        }
        //If it doesn't apply to all then it must apply to only the first attack
        else {
            finalAttack[0] += featList[intChosen[i]].attackMod;
            finalDamage[0] += featList[intChosen[i]].damageMod;
        }
        finalRangePenalty += featList[intChosen[i]].rangePenaltyMod;

        //Print out messages based on special attack modifications from chosen feat and apply modifications
        switch (featList[intChosen[i]].specialAttackMod){
            case EXTRA_ARROW:
                printf("Your first attack shoots two arrows. Roll damage and add the flat damage bonuses for both. Sneak attack and critial hits only apply for one arrow.\n");
                break;
            case EXTRA_ATTACK_HIGHEST:
                printf("You have two shots at the highest bonus for this round. (Hence the extra listed attack.)\n");
                numAttacks++;
                break;
            default:
                break;
        }
    }

    //Make sure they don't have more attacks than the software is able to handle (unlikely, but possible)
    if (numAttacks > MAXATTACKS){
        printf("ERROR: contact program developer. You have found a feat combination that creates more attacks per round than this program can handle.\n");
        return -1;
    }

    //Add base attack bonus and ability bonuses
    if (numAttacks <= baseNumAttacks){
        for (int i = 0; i < numAttacks; i++){
            finalAttack[i] += baseAttackBonus[i];
            finalAttack[i] += attackAbilityMod;
            finalDamage[i] += damageAbilityMod;
        }
    }
    else {
        for (int i = 0, j = 0; i < numAttacks; i++, j++){
            finalAttack[i] += baseAttackBonus[j];
            finalAttack[i] += attackAbilityMod;
            finalDamage[i] += damageAbilityMod;
            if (i == 0 && j == 0){
                i++;
                finalAttack[i] += baseAttackBonus[j];
                finalAttack[i] += attackAbilityMod;
                finalDamage[i] += damageAbilityMod;
            }

        }
    }

    //Print results, assuming there is at least one attack
    printf("Attack bonuses: ");
    printf("%i", finalAttack[0]);
    for (int i = 1; i < numAttacks; i++){
        printf("/%i", finalAttack[i]);
    }
    printf("\nDamage bonuses: ");
    printf("%i", finalDamage[0]);
    for (int i = 1; i < numAttacks; i++){
        printf("/%i", finalDamage[i]);
    }
    printf("\n");
    printf("Range penalty for each increment after the first is %i\n", finalRangePenalty);


	printf("Enter any key to close: (keeps Windows from making this disappear before you can see it) ");
	tempChar = getchar();
    return 0;
} //End of main

//Turns an array of characters, separated by a specific character, into an array of integers. intArr must be long enough to hold the result.
//THIS VERSION EDITED FOR COMPATIBILITY WITH VISUAL STUDIOS
void charArrayToIntArray(char charArr[], int intArr[], char separator, int acceptNewline){
    #ifndef TRUE
    #define TRUE 1
    #endif
    int start, stop, place, i;
    start = stop = place = i = 0;
    int subLen = 0;
    for (i = 0; charArr[i] != '\0'; i++){
        //If the character is a separator, or the newlines are accepted as separators and the char is a newline or carriage return,
        //then get the substring and convert it to an integer
        if (charArr[i] == separator || (acceptNewline == TRUE && (charArr[i] == '\n' || charArr[i] == '\r'))){
            stop = i;
            if (start < stop){
                char temp[MAXLENGTH];
                subLen = copySubStr(temp, charArr, start, stop, stop);
                //printf("Sublen: %i start: %i stop %i\n", subLen, start, stop);
                if (subLen > 0){
                    intArr[place++] = atoi(temp);
                    start = stop;
                }
                else {
                    //printf("DEBUG: error: copySubStr did not return the proper value\n");
                }
            }
        }
        //Only change start if a value has been copied already (signified by start being the same as stop)
        if (isdigit(charArr[i]) && start == stop){
            start = i;
        }
    }

    intArr[place] = '\0';
    return;
}

//Print menu of possible feats
void printMenu(char selected[]){
    for (int i = 0; i < FINAL_VALUE; i++){
        printf("%i. %s\n", i + 1, featList[i].name);
    }
    printf("Please enter, on one line, the numbers of the feats you want to apply to this roll, separated by spaces: ");

    myGetline(selected, MAXLENGTH, TRUE);
}

//Puts the data into the feats.
void loadFeats(int bab){
    int temp;

    //Deadly Aim
    strcpy(featList[DEADLY_AIM].name, "Deadly Aim\0");
    featList[DEADLY_AIM].applyToAll = TRUE;
    featList[DEADLY_AIM].specialAttackMod = NONE;
    temp = (int) round(bab / 4);
    featList[DEADLY_AIM].attackMod = (temp > 1) ? temp * -1 : -1;
    featList[DEADLY_AIM].damageMod = (temp > 1) ? temp * 2 : 2;
    featList[DEADLY_AIM].rangePenaltyMod = 0;
    featList[DEADLY_AIM].attackType = RANGED;

    //Farshot
    strcpy(featList[FARSHOT].name, "Farshot\0");
    featList[FARSHOT].applyToAll = TRUE;
    featList[FARSHOT].specialAttackMod = 0;
    featList[FARSHOT].attackMod = 0;
    featList[FARSHOT].damageMod = 0;
    featList[FARSHOT].rangePenaltyMod = +1;
    featList[FARSHOT].attackType = RANGED;

    //Manyshot
    strcpy(featList[MANYSHOT].name, "Manyshot\0");
    featList[MANYSHOT].applyToAll = FALSE;
    featList[MANYSHOT].specialAttackMod = EXTRA_ARROW;
    featList[MANYSHOT].attackMod = 0;
    featList[MANYSHOT].damageMod = 0;
    featList[MANYSHOT].rangePenaltyMod = 0;
    featList[MANYSHOT].attackType = RANGED;

    //Point-Blank Shot
    strcpy(featList[POINT_BLANK_SHOT].name, "Point-Blank Shot\0");
    featList[POINT_BLANK_SHOT].applyToAll = TRUE;
    featList[POINT_BLANK_SHOT].specialAttackMod = NONE;
    featList[POINT_BLANK_SHOT].attackMod = 1;
    featList[POINT_BLANK_SHOT].damageMod = 1;
    featList[POINT_BLANK_SHOT].rangePenaltyMod = 0;
    featList[POINT_BLANK_SHOT].attackType = RANGED;

    //Rapid Shot
    strcpy(featList[RAPID_SHOT].name, "Rapid Shot\0");
    featList[RAPID_SHOT].applyToAll = TRUE;
    featList[RAPID_SHOT].specialAttackMod = EXTRA_ATTACK_HIGHEST;
    featList[RAPID_SHOT].attackMod = -2;
    featList[RAPID_SHOT].damageMod = 0;
    featList[RAPID_SHOT].rangePenaltyMod = 0;
    featList[RAPID_SHOT].attackType = RANGED;

    //End marker
    featList[FINAL_VALUE].name[0] = '\0';
} //End of loadFeats

//Get a line of input from the keyboard
int myGetline(char s[], int maxLen, int putNl){
	#ifndef TRUE
	#define TRUE 1
	#endif

	int i, c;

	for (i = 0; i < maxLen-1 && (c = getchar()) != '\n'; ++i){
		s[i] = c;
    }
	if ((c == '\n') && (putNl == TRUE))
	{
		s[i] = c;
		++i;
	}
	s[i] = '\0';
	return i;
} //end of myGetline
//Copies a substring from src to dest using subStart and subEnd as indexes. Copies at most maxLen chars, not including the NULL pointer
int copySubStr(char dest[], char src[], int srcStart, int srcEnd, int maxLen){
    int iter, i;

    //Make sure start is less than end
    if (srcStart > srcEnd)
        return -1;

    //Make sure end is less than maxLen
    if (srcEnd > maxLen)
        return -2;

    //If srcEnd is -1, go to the end of the array; strlen does not include the null character in its length calculation
    if (srcEnd == -1)
        srcEnd = strlen(src);

    //Copy substring bounded by srcStart and srcEnd to dest
    for (iter = srcStart, i = 0; iter <= srcEnd && src[iter] != '\0' && i < maxLen; iter++, i++){
        dest[i] = src[iter];
    }

    dest[i] = '\0';

    //Return length of dest
    return i;
}
