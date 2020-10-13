/* Don't run those old documents (pre-census_writer_3.c) on the new census. They will not work and just say too many objects.*/
//Now makes pages of 4096 objects instead of 2048!

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

FILE *census;
FILE *output;
FILE *names;
FILE *links;

void ErrorMsg() {
    fflush(output);
    fprintf(output, "There was an error.\n");
}

int GoToNextDoubleQuote() {
    for (;;) {
        int next = getc(census);
        if (next == EOF) {
			return 0;
		}
        if (next == '"') return 1;
    }
}

int ScanToNextDoubleQuote(char *token) {
    char *last = token + 335;
    *token = '\0';
    for (;;) {
        int next = getc(census);
        if (next == EOF) {
			return 0;
		}
        if (next == '"') {
            *token = '\0';
            return 1;
        }
        if (token == last) {
            printf("Token too long.\n");
            return 0;
        }
        *token = (char) next;
        token++;
    }
}

int GetToken(char *token) {
    return GoToNextDoubleQuote() && ScanToNextDoubleQuote(token);
}

int IsWikiArticle(char *name) {
    rewind(links);
    int strPos;
    char ch = getc(links);
    char readName[257];
    while (ch != EOF) {
        strPos = 0;
        while (ch != EOF && ch != '\n') {
            if (strPos == 256) break;
            readName[strPos] = ch;
            strPos++;
            ch = getc(links);
        }
        readName[strPos] = '\0';
        if (!strcmp(name, readName)) return 1;
        if (strcmp(name, readName) < 0) return 0;
        if (ch != EOF) ch = getc(links);
    }
    return 0;
}

char *ScanName(char *apgcode, char *commonNameBuffer) {
    //names = fopen("names.txt", "r");
    rewind(names);
    int strPos;
    char ch = getc(names);
    char code[334];
    while (ch != EOF) {
        strPos = 0;
        while (ch != EOF && ch != ' ') {
            if (strPos == 333) break;
            code[strPos] = ch;
            strPos++;
            ch = getc(names);
        }
        code[strPos] = '\0';
        strPos = 0;
        ch = getc(names);
        while (ch != EOF && ch != '\n') {
            if (strPos == 256) break;
            commonNameBuffer[strPos] = ch;
            strPos++;
            ch = getc(names);
        }
        commonNameBuffer[strPos] = '\0';
        if (!strcmp(code, apgcode)) {
            return commonNameBuffer;
        }
        if (ch != EOF) ch = getc(names);
    }
    return "";
}

void ReplaceSpaces(char *name, char *nameBuffer) {
    int index = 0;
    while (name[index]) {
        if (name[index] == ' ') nameBuffer[index] = '_';
        else nameBuffer[index] = name[index];
        index++;
    }
    nameBuffer[index] = '\0';
}

int CountLines() {
	FILE *censusCopy = fopen("census.txt", "r");
	int linenum = 0;
	char ch;
	while (1) {
		ch = getc(censusCopy);
		if (ch == EOF || ch == '\n') {
			linenum++;
			if (ch == EOF) break;
		}
	}
	fclose(censusCopy);
	return linenum;
}

int MakeList() {
	int objNum = 0;
	int isDone;
    int rank = 0;
    char *name;
    char commonName[257];
    char apgcode[336];
    char count[336] = "0";
    char prevCount[336];
    while (1) {
		objNum++;
        isDone = !GetToken(apgcode);
        while (!strcmp(apgcode, "apgcode") && !strcmp(apgcode, "xs20_gbb8r2qkz11") && !strcmp(apgcode, "xs18_5b8r2qr") && !strcmp(apgcode, "xs19_25b8r2qr") && !strcmp(apgcode, "xs20_o4q2r8brz01") && !strcmp(apgcode, "xs20_g4q2r8brz11") && !strcmp(apgcode, "xs20_rb8r2qkz011") && !strcmp(apgcode, "xs21_69f0rbz03521") && !strcmp(apgcode, "xs22_69f0rbz25521") && !strcmp(apgcode, "xs23_69a4oz69f0dd") && !strcmp(apgcode, "xs21_09f0rbz69521") && !strcmp(apgcode, "xs23_3hu0mmzcia43") && !strcmp(apgcode, "yl144_1_32_580daec726a1a769e26066366554d6e5") && !strcmp(apgcode, "yl1152_1_305_5795153c1f2b6ae9bfe120246730d361") && !strcmp(apgcode, "yl144_1_32_54a1c82db27c03fc332b1c87fa1f6a0a") && !strcmp(apgcode, "")) {
            if (!strcmp(apgcode, "apgcode")) printf("Invalid line automatically removed from text file\n");
            isDone = !GetToken(count);
            isDone = !GetToken(apgcode);
        }
        if (objNum != 1 && (isDone || objNum % 4096 == 1)) {
            fprintf(output, "</table>\n");
            if (objNum > 4096) {
                fprintf(output, "<a href=\"/catagolue-backup-%d\">Previous page</a>", objNum / 4096 - 1);
                if (!isDone) fprintf(output, " - ");
            }
            if (!isDone) fprintf(output, "<a href=\"/catagolue-backup-%d\">Next page</a>\n\n\n", objNum / 4096 + 1);
            else {
                printf("%d objects found.\n", objNum);
                return 0;
            }
        }
        if (objNum % 4096 == 1) {
            fprintf(output, "<h2>Backup of Catagolue b3s23/C1 census</h2>\n");
            fprintf(output, "<h4>Page %d</h4>\n", objNum / 4096 + 1);
            fprintf(output, "<p>Disclaimer: This is a fan-generated backup. It has no affiliation with Catagolue in any way.</p>\n");
            fprintf(output, "<table>\n");
            fprintf(output, "<tr><td><b>Rank</b></td><td><b>Name</b></td><td><b>apgcode</b></td><td><b>Count</b></td>\n");
        }
        name = ScanName(apgcode, commonName);
        int hasArticle = IsWikiArticle(name);
        char uRLName[64];
        if (hasArticle) ReplaceSpaces(name, uRLName);
        strcpy(prevCount, count);
        isDone = !GetToken(count);
        if (isDone) {
            printf("Error: File ended on apgcode, not count.\n");
            return 1;
        }
        if (strcmp(prevCount, count)) rank = objNum;
        fprintf(output, "<tr><td>");
        if (rank == objNum || objNum % 4096 == 1) fprintf(output, "%d", rank);
        fprintf(output, "</td><td>");
        if (hasArticle) {
            fprintf(output, "<a href = \"http://conwaylife.com/wiki/%s\">%s</a>", uRLName, name);
        }
        else {
            fprintf(output, "%s", name);
        }
        fprintf(output, "</td><td><a href=\"http://catagolue.appspot.com/object/%s/b3s23\">%s</a></td><td>%s<br></td></tr>\n", apgcode, apgcode, count);
        fflush(output);
        //free(apgcode);
        //free(name);
        //free(count);
    }
    printf("Too many objects.\n");
    ErrorMsg();
    return 1;
}

int main(void) {
    census = fopen("census.txt", "r");
    output = fopen("result.html", "w");
    names = fopen("names.txt", "r");
    links = fopen("links.txt", "r");
    int result = MakeList();
    if (!result) printf("File is finished.\n");
    getchar();
    fclose(census);
    fclose(output);
    fclose(names);
    return 0;
}
