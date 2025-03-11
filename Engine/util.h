#pragma once

#include <iostream>

// Essayer en envoyant le code d'erreur comme r�sultat
// Il ne faut pas oublier de "rattraper" le code...
template <class Type>
void DXEssayer(const Type& Resultat)
{
    if (Resultat != S_OK)
    {
        throw Resultat;
    }
}

// Plus pratique, essayer en envoyant un code sp�cifique
// comme r�sultat
template <class Type1, class Type2>
void DXEssayer(const Type1& Resultat, const Type2& unCode)
{
    if (Resultat != S_OK)
    {
        throw unCode;
    }
}

// Valider un pointeur
template <class Type>
void DXValider(const void* UnPointeur, const Type& unCode)
{
    if (UnPointeur == nullptr)
    {
        throw unCode;
    }
}

// Rel�cher un objet COM (un objet DirectX dans notre cas)
template <class Type>
void DXRelacher(Type& UnPointeur)
{
    if (UnPointeur != nullptr)
    {
        UnPointeur->Release();
        UnPointeur = nullptr;
    }
}
