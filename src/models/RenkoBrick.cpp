#include "models/RenkoBrick.h"

json RenkoBrick::to_json() const {
    return {
        {"open", open},
        {"close", close},
        {"dir", dir},
        {"ts", ts}
    };
}