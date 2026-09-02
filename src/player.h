#if !defined(_INCLUDE_PLAYER_H_)
#define _INCLUDE_PLAYER_H_

#define MAXPLAYERS 64

class CPlayer
{
public:
    inline void SetIsInGame(bool value)
    {
        m_bIsInGame = value;
    }
    inline bool IsInGame()
    {
        return m_bIsInGame;
    }

private:
    bool m_bIsInGame = false;
};

inline CPlayer m_vecPlayer[MAXPLAYERS];

inline void AddPlayer(CPlayerSlot slot)
{
    if (!slot.IsValid())
        return;

    m_vecPlayer[slot.Get()] = CPlayer();
}

inline CPlayer* GetPlayer(CPlayerSlot slot)
{
    if (!slot.IsValid())
        return nullptr;

    return &m_vecPlayer[slot.Get()];
}

inline CPlayer* GetPlayer(CEntityInstance* pController)
{
    if (!pController)
        return nullptr;

    return &m_vecPlayer[pController->GetEntityIndex().Get() - 1];
}

#endif // _INCLUDE_PLAYER_H_
