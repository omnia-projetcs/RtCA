//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
//supprimer tout le contenu
void RichEditInit(HWND HRichEdit)
{
  CHARFORMAT2 Format;

  //init de la zone de texte
  ZeroMemory(&Format, sizeof(CHARFORMAT2));
  Format.cbSize = sizeof(CHARFORMAT2);
  Format.dwMask = CFM_BOLD | CFM_COLOR | CFM_ITALIC | CFM_SIZE | CFM_UNDERLINE;
  Format.yHeight = 200; //taille du texte
  Format.yOffset = 0;
  Format.crTextColor = RGB(0,0,0);   //Couleur du texte
  Format.crBackColor = RGB(255,255,255);   //Couleur du fond
  Format.bCharSet = ANSI_CHARSET;
  Format.bPitchAndFamily = DEFAULT_PITCH;
  strcpy(Format.szFaceName,"MS Sans Serif");
  SendMessage(HRichEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM) &Format);   //appliquer le format a tout le composant
  SendMessage(HRichEdit, WM_SETTEXT, 0, (LPARAM)"");//on vide le texte
}
//------------------------------------------------------------------------------
//texte simple
void RichEditCouleur(HWND HRichEdit,COLORREF couleur,char* txt)
{
    CHARFORMAT2 Format; //format du texte d'un richedit
    CHARRANGE Selection; //sélection

    ZeroMemory(&Format, sizeof(CHARFORMAT2));
    Format.cbSize = sizeof(CHARFORMAT2);
    Format.dwMask = CFM_CHARSET|CFM_COLOR|CFM_UNDERLINE|CFM_ITALIC|CFM_BOLD|CFM_SIZE;
    Format.crTextColor = couleur;
    Format.dwEffects = 0;
    Format.yHeight = 200;

    //récupération de la taille du texte contenu, on ajoute le texte après
    GETTEXTLENGTHEX TxtLenfth;
    TxtLenfth.codepage=CP_ACP;
    TxtLenfth.flags=GTL_NUMCHARS;
    unsigned int pos= SendMessage(HRichEdit,EM_GETTEXTLENGTHEX,(WPARAM)&TxtLenfth,0);

    Selection.cpMin = pos;
    Selection.cpMax = pos+strlen(txt);

    SendMessage(HRichEdit, EM_EXSETSEL, 0, (LPARAM) &Selection);
    SendMessage(HRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &Format);
	// Il ne reste plus qu'à insérer le texte formaté:
    SendMessage(HRichEdit, EM_REPLACESEL, 0, (LPARAM)(LPCTSTR) txt);
}
//------------------------------------------------------------------------------
//En haut d ela fenêtre
void RichSetTopPos(HWND HRichEdit)
{
  /*POINT p;
  p.x = 0;
  p.y = 0;
  SendMessage(HRichEdit,EM_SETSCROLLPOS,(WPARAM)0,(LPARAM) &p);*/
  /*CHARRANGE c;
  memset(&c,0,sizeof(CHARRANGE));
  c.cpMax = 1;

  SendMessage(HRichEdit, EM_EXSETSEL, 0, &c);*/
  SendMessage(HRichEdit, WM_VSCROLL, SB_BOTTOM, 0);
  //SetScrollPos(HRichEdit, SB_VERT, 0, TRUE);
}
//------------------------------------------------------------------------------
//Ajout de texte colorer et gras au RichEdit ; couleur = RGB(255, 0, 0)
void RichEditCouleurGras(HWND HRichEdit,COLORREF couleur,char* txt)
{
    CHARFORMAT2 Format; //format du texte d'un richedit
    CHARRANGE Selection; //sélection

    ZeroMemory(&Format, sizeof(CHARFORMAT2));
    Format.cbSize = sizeof(CHARFORMAT2);
    Format.dwMask = CFM_COLOR|CFM_CHARSET|CFM_UNDERLINE|CFM_ITALIC|CFM_BOLD|CFM_SIZE;
    Format.crTextColor = couleur;
    Format.dwEffects = CFE_BOLD;
    Format.yHeight = 200;

    //récupération de la taille du texte contenu, on ajoute le texte après
    GETTEXTLENGTHEX TxtLenfth;
    TxtLenfth.codepage=CP_ACP;
    TxtLenfth.flags=GTL_NUMCHARS;
    unsigned int pos= SendMessage(HRichEdit,EM_GETTEXTLENGTHEX,(WPARAM)&TxtLenfth,0);

    Selection.cpMin = pos;
    Selection.cpMax = pos+strlen(txt);

    SendMessage(HRichEdit, EM_EXSETSEL, 0, (LPARAM) &Selection);
    SendMessage(HRichEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM) &Format);
	// Il ne reste plus qu'à insérer le texte formaté:
    SendMessage(HRichEdit, EM_REPLACESEL, 0, (LPARAM)(LPCTSTR) txt);
}

//------------------------------------------------------------------------------
//get richedit text length
DWORD RichEditTextSize(HWND HRichEdit)
{
  GETTEXTLENGTHEX getLen;
  getLen.flags    = GTL_DEFAULT;//texte size
  getLen.codepage = CP_ACP;     //ANSI

  DWORD size = SendMessage(HRichEdit, EM_GETTEXTLENGTHEX , (WPARAM) &getLen, 0);
  if (size != E_INVALIDARG) return size;
  else return 0;
}
