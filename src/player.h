#if !defined(_INCLUDE_PLAYER_H_)
#define _INCLUDE_PLAYER_H_

#define MAXPLAYERS 64

class CPlayer
{
public:
    inline void SetInGame(bool value)
    {
        m_bIsGame = value;
    }
    inline bool IsInGame()
    {
        return m_bIsGame;
    }
    inline void SetIsFullConnect(bool value)
    {
        m_bIsFullConnect = value;
    }
    inline bool IsFullConnect()
    {
        return m_bIsFullConnect;
    }

private:
    bool m_bIsGame;
    bool m_bIsFullConnect;
};

inline CPlayer m_vecPlayer[MAXPLAYERS];

inline void AddPlayer(int slot)
{
    m_vecPlayer[slot] = CPlayer();
}

inline CPlayer& GetPlayer(int slot)
{
    return m_vecPlayer[slot];
}

inline int GetPlayerSlot(CEntityInstance* pController)
{
    return pController->GetEntityIndex().Get() - 1;
}

#endif // _INCLUDE_PLAYER_H_
