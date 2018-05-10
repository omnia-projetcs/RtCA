//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addProxyConfig()
{
  if (IsDlgButtonChecked(h_proxy,PROXY_CHK_USE)==BST_CHECKED)
  {
    //if login and password
    if ((GetWindowTextLength(GetDlgItem(h_proxy,PROXY_ED_USER)) > 0) ||
        (GetWindowTextLength(GetDlgItem(h_proxy,PROXY_ED_PASSWORD)) > 0))
    {
      use_proxy_advanced_settings = TRUE;
      GetWindowText(GetDlgItem(h_proxy,PROXY_ED_USER),proxy_ch_user,DEFAULT_TMP_SIZE);
      GetWindowText(GetDlgItem(h_proxy,PROXY_ED_PASSWORD),proxy_ch_password,DEFAULT_TMP_SIZE);
    }else
    {
      proxy_ch_user[0]            = 0;
      proxy_ch_password[0]        = 0;
      use_proxy_advanced_settings = FALSE;
    }

    //other proxy ?
    if (IsDlgButtonChecked(h_proxy,PROXY_CHK_AUTOCONF)==BST_CHECKED)
    {
      if (GetWindowTextLength(GetDlgItem(h_proxy,PROXY_ED_PROXY)) > 0)
      {
        use_other_proxy             = TRUE;
        char tmp[DEFAULT_TMP_SIZE];
        GetWindowText(GetDlgItem(h_proxy,PROXY_ED_PROXY),tmp,DEFAULT_TMP_SIZE);
        snprintf(proxy_ch_auth,DEFAULT_TMP_SIZE,"http=%s",tmp);
      }else
      {
        use_other_proxy             = FALSE;
        proxy_ch_auth[0]            = 0;
      }
    }else
    {
      use_other_proxy             = FALSE;
      proxy_ch_auth[0]            = 0;
    }
  }else
  {
    proxy_ch_user[0]            = 0;
    proxy_ch_password[0]        = 0;
    proxy_ch_auth[0]            = 0;
    use_proxy_advanced_settings = FALSE;
    use_other_proxy             = FALSE;
  }
}
//------------------------------------------------------------------------------
BOOL CALLBACK DialogProc_proxy(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
    case WM_COMMAND:
      switch (HIWORD(wParam))
      {
        case BN_CLICKED:
          switch(LOWORD(wParam))
          {
            case PROXY_CHK_USE:
              if (IsDlgButtonChecked(hwnd,PROXY_CHK_USE)==BST_CHECKED)
              {
                EnableWindow(GetDlgItem((HWND)hwnd,PROXY_CHK_AUTOCONF),TRUE);
                EnableWindow(GetDlgItem((HWND)hwnd,PROXY_ED_USER),TRUE);
                EnableWindow(GetDlgItem((HWND)hwnd,PROXY_ED_PASSWORD),TRUE);

                if (IsDlgButtonChecked(hwnd,PROXY_CHK_AUTOCONF)==BST_CHECKED) EnableWindow(GetDlgItem((HWND)hwnd,PROXY_ED_PROXY),FALSE);
                else EnableWindow(GetDlgItem((HWND)hwnd,PROXY_ED_PROXY),TRUE);
              }else
              {
                EnableWindow(GetDlgItem((HWND)hwnd,PROXY_CHK_AUTOCONF),FALSE);
                EnableWindow(GetDlgItem((HWND)hwnd,PROXY_ED_USER),FALSE);
                EnableWindow(GetDlgItem((HWND)hwnd,PROXY_ED_PASSWORD),FALSE);
                EnableWindow(GetDlgItem((HWND)hwnd,PROXY_ED_PROXY),FALSE);
              }
            break;
            case PROXY_CHK_AUTOCONF:
              if (IsDlgButtonChecked(hwnd,PROXY_CHK_AUTOCONF)==BST_CHECKED) EnableWindow(GetDlgItem((HWND)hwnd,PROXY_ED_PROXY),FALSE);
              else EnableWindow(GetDlgItem((HWND)hwnd,PROXY_ED_PROXY),TRUE);
            break;
            case PROXY_BT_OK:
              addProxyConfig();
              ShowWindow(hwnd, SW_HIDE);
            break;
          }
        break;
      }
    break;
    case WM_CLOSE :
      addProxyConfig();
      ShowWindow(hwnd, SW_HIDE);
    break;
  }
  return FALSE;
}
