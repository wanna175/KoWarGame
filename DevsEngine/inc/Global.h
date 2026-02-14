// ********************************************************************
// * 헤더정의: Global.h
// * 설    명: 전역상수를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 12
// ********************************************************************
#pragma once

#include <string>
#include <TypeDef.h>

using std::string;

// ********************************************************************
// * 설    명: ArtilleryModel의 포트, 컨텐트, state를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 12
// ********************************************************************
namespace ModelConfig{
    namespace Port{
        const string P_art_IN  = "IN";
        //TODO : 외부 아웃풋 포트를 이렇게 많이 정의할 필요가 있을까?? 고민
        //prefix! 뒤에 오브젝트 아이디를 붙여서 완성한다.
        const string P_art_OUT                = "OUT";
        const string P_art_MOVED_OUTPUT       = "MOVED_OUTPUT";
        const string P_art_ATTACKED_OUTPUT    = "ATTACKED_OUTPUT";
        const string P_art_DESTROY_OUTPUT     = "DESTROY_OUTPUT";
    }
    namespace Content{
        const string C_art_ORDER_IN_SALLY  = "ORDER_IN_SALLY";
        const string C_art_ATTACKED        = "ART_ATTACKED";
        const string C_art_DESTROY         = "ART_DESTROY";
        const string C_art_ORDER_IN_ATTACK = "ORDER_IN_ATTACK";
        const string C_art_MOVED           = "ART_MOVED_OUTPUT";
    }
    namespace State{
        const string S_art_WAIT_IN  = "WAIT_IN";
        const string S_art_WAIT_OUT = "WAIT_OUT";
        const string S_art_MOVE     = "MOVE";
        const string S_art_ATTACK   = "ATTACK";
        const string S_art_ATTACKED = "ATTACKED";
        const string S_art_DESTROY  = "DESTROY";
    }
}
// ********************************************************************
// * 설    명: MissileModel의 포트, 컨텐트, state를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 09. 04
// ********************************************************************
namespace ModelConfig{
    namespace Port{
        const string P_mis_IN  = "IN";
        //TODO : 외부 아웃풋 포트를 이렇게 많이 정의할 필요가 있을까?? 고민
        //prefix! 뒤에 오브젝트 아이디를 붙여서 완성한다.
        const string P_mis_OUT                = "OUT";
        const string P_mis_MOVED_OUTPUT       = "MOVED_OUTPUT";
        const string P_mis_DESTROY_OUTPUT     = "DESTROY_OUTPUT";
    }
    namespace Content{
        const string C_mis_ORDER_IN_ATTACK = "ORDER_IN_ATTACK";
        const string C_mis_MOVED           = "MIS_MOVED_OUTPUT";
        const string C_mis_DESTROY         = "MISSILE_BOMB";
    }
    namespace State{
        const string S_mis_WAIT     = "WAIT";
        const string S_mis_MOVE     = "MOVE";
        const string S_mis_DESTROY  = "DESTROY";
    }
}

// ********************************************************************
// * 설    명: MilitaryUnitModel의 포트, 컨텐트, state를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 12
// ********************************************************************
namespace ModelConfig{
    namespace Port{
        const string P_mil_IN  = "IN";
        //prefix! 뒤에 오브젝트 아이디를 붙여서 완성한다.
        const string P_mil_OUT             = "OUT";
        const string P_mil_ATTACKED_OUTPUT = "ATTACKED_OUTPUT";
        const string P_mil_DESTROY_OUTPUT  = "DESTROY_OUTPUT";
    }
    namespace Content{
        const string C_mil_ORDER_IN_SALLY = "ORDER_IN_SALLY";
        const string C_mil_ATTACKED       = "MIL_ATTACKED";
        const string C_mil_DESTROY        = "MIL_DESTROY";
    }
    namespace State{
        const string S_mil_WAIT     = "WAIT";
        const string S_mil_ORDER    = "ORDER";
        const string S_mil_ATTACKED = "ATTACKED";
        const string S_mil_DESTROY  = "DESTROY";
    }
}

// ********************************************************************
// * 설    명: UnitMapModel의 포트, 컨텐트, state를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 12
// ********************************************************************
namespace ModelConfig{
    namespace Port{
        const string P_map_IN    = "IN";
        //prefix! 뒤에 오브젝트 아이디를 붙여서 완성한다.
        const string P_map_OUT   = "MAP_OUT";
    }
    namespace Content{
        
    }
    namespace State{
        const string S_map_WAIT  = "WAIT";
    }
}

// ********************************************************************
// * 설    명: TimeControlModel의 포트, 컨텐트, state를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 12
// ********************************************************************
namespace ModelConfig{
    namespace Port{
        const string P_tic_TIMEOUT = "TIME_OUT";
    }
    namespace Content{
        const string C_tic_TIMEOUT = "TIME_OUT";
    }
    namespace State{
        const string S_tic_WAIT  = "WAIT";
    }
}

// ********************************************************************
// * 설    명: MilitaryCoupledModel의 포트, 컨텐트, state를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 12
// ********************************************************************
namespace ModelConfig{
    namespace Port{
        const string P_mic_IN  = "IN";
        const string P_mic_OUT = "OUT"; 
    }
    namespace Content{
        
    }
    namespace State{
        
    }
}

// ********************************************************************
// * 설    명: MainCoupledModel의 포트, 컨텐트, state를 정의한다.
// * 작 성 자: KJH
// * 작성날짜: 2025. 08. 12
// ********************************************************************
namespace ModelConfig{
    namespace Port{
        const string P_main_IN  = "EXTERNAL_INPUT";
        const string P_main_OUT = "EXTERNAL_OUTPUT";
    }
    namespace Content{
        
    }
    namespace State{
        
    }
}