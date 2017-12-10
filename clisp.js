Broom['autoAlloc:'](true)
import ['Library/Control/Applicative']()["Env", "GlobalEnv", "Truth", "_eval", "parse", "tokenize", "read_from", "atom", "to_string", "repl"]['each:']((function(i, v) {
    Reflect['addGlobalVariable:'](v)
}))
Env = Object['new']()
Env['on:do:']("initParams:args:outer:", (function(params, args, outer) {
    (this.outer) = outer;
    (this.dict) = Map['new']();
    params['toBoolean']()['&'](args)['ifTrue:']((function() {
        params['count']()['times:']((function(i) {
            this['put:at:']((args['at:'](i)), (params['at:'](i)))
        }))
    }))
}))
Env['on:do:']("at:", (function(k) {
    return this.dict['at:']('$' ['+'](k))
}))
Env['on:do:']("put:at:", (function(v, k) {
    this.dict['put:at:'](v, '$' ['+'](k))
}))
Env['on:do:']("find:", (function(k) {
    outer = (this.outer);
    key = '$' ['+'](k);
    return this.dict['at:'](key)['isNil']()['either:or:']((function() {
        return outer['isNil']()['either:or:'](Nil, (function() {
            return outer['find:'](k)
        }))
    }), me)
}))
Env['on:do:']("toString", (function() {
    return '(' ['+'](this.dict['toString']())['+'](") < ")['+']((this.outer['toString']()))
}))
GlobalEnv = Env['new']()['initParams:args:outer:']([], [], NULL)
GlobalInit = Map['new']()
GlobalInit['do']()['put:at:']((function(a, b) {
    return a['+'](b)
}), "+")['put:at:']((function(a, b) {
    return a['-'](b)
}), "-")['put:at:']((function(a, b) {
    return a['*'](b)
}), "*")['put:at:']((function(a, b) {
    return a['/'](b)
}), "/")['put:at:']((function(a, b) {
    return a['toBoolean']()['&'](b)
}), "and")['put:at:']((function(a, b) {
    return a['toBoolean']()['|'](b)
}), "or")['put:at:']((function(a) {
    return a['toBoolean']()['not']()
}), "not")['put:at:']((function(a, b) {
    return a['>'](b)
}), ">")['put:at:']((function(a, b) {
    return a['<'](b)
}), "<")['put:at:']((function(a, b) {
    return a['>=:'](b)
}), ">=")['put:at:']((function(a, b) {
    return a['<=:'](b)
}), "<=")['put:at:']((function(a, b) {
    return a['='](b)
}), "=")['put:at:']((function(a, b) {
    return a['='](b)
}), "equal?")['put:at:']((function(a, b) {
    return a['='](b)
}), "eq?")['put:at:']((function(a) {
    return a['count']()
}), "length")['put:at:']((function(a, b) {
    return [a, b]
}), "cons")['put:at:']((function(a) {
    return a['at:'](0)
}), "car")['put:at:']((function(a) {
    return a['at:'](1)
}), "cdr")['put:at:']((function(a, b) {
    return a['+'](b)
}), "append")['put:at:']((function(varargs) {
    return args
}), "list")['put:at:']((function(a) {
    return Reflect['isObject:linkedTo:'](a, Array)
}), "list?")['put:at:']((function(a) {
    return Reflect['isObject:linkedTo:'](a, Array)['&']((a['count']()['='](0)))
}), "null?")['put:at:']((function(a) {
    return Reflect['isObject:linkedTo:'](a, String)
}), "symbol?")['put:at:']((function(a) {
    return ((Reflect['isObject:linkedTo:'](a, String)))['|']((Reflect['isObject:linkedTo:'](a, Number)))['|']((a['isNil']()))
}), "atom?")['put:at:']((function() {
    return atom['applyTo:']((Program['getCharacter']()))
}), "read")['done']()
GlobalInit['each:']((function(k, v) {
    GlobalEnv['put:at:'](v, k)
}))["abs", "atan", "ceil", "cos", "exp", "floor", "log", "round", "sin", "sqrt", "tan"]['each:']((function(i, v) {
    GlobalEnv['put:at:']((Number['&method:'](v)), v)
}))
GlobalEnv['put:at:']((Number['&method:']("toPowerOf:")), "pow")
GlobalEnv['put:at:']((Number['&method:']("max:")), "max")
GlobalEnv['put:at:']((Number['&method:']("min:")), "min")
Truth = (function(st) {
    return Reflect['isObject:linkedTo:'](st, Array)['either:or:']((st['count']()['toBoolean']()), st)
})
Truth['on:do:']("of:", (function(exp) {
    return this['applyTo:'](exp)
}))
_eval = (function(x, en) {
    env = en['or:'](GlobalEnv);
    ((Reflect['isObject:linkedTo:'](x, String)))['ifTrue:']((function() {
        return env['find:'](x)['at:'](x)
    }));
    ((Reflect['isObject:linkedTo:'](x, Array)))['ifFalse:']((function() {
        return x
    }));
    ((x['count']()['='](0)))['ifTrue:']((function() {
        return x
    }));
    ((x['@'](0)['=']("quote")))['|']((x['@'](0)['=']("'")))['ifTrue:']((function() {
        return x['at:'](1)
    }));
    ((x['@'](0)['=']("if")))['ifTrue:']((function() {
        return _eval['applyTo:and:']((Truth['of:']((_eval['applyTo:and:'](x['@'](1), env)))['either:or:'](x['@'](2), x['@'](3))), env)
    }));
    ((x['@'](0)['=']("set!")))['ifTrue:']((function() {
        env['find:'](x['@'](1))['put:at:']((_eval['applyTo:and:'](x['@'](2), env)), x['@'](1));
        return env['at:'](x['@'](1))
    }));
    ((x['@'](0)['=']("define")))['ifTrue:']((function() {
        env['put:at:']((_eval['applyTo:and:'](x['@'](2), env)), x['@'](1));
        return env['at:'](x['@'](1))
    }));
    ((x['@'](0)['=']("lambda")))['ifTrue:']((function() {
        return (function(varargs) {
            return _eval['applyTo:and:'](this.x['@'](2), (Env['new']()['initParams:args:outer:'](this.x['@'](1), args, (this.env))))
        })
    }));
    ((x['@'](0)['=']("begin")))['ifTrue:']((function() {
        val = NULL;
        x['each:']((function(i, v) {
            val = _eval['applyTo:and:'](x['@'](i), env)
        }));
        return val
    }));
    ((x['@'](0)['=']("#")))['ifTrue:']((function() {
        return ((x['from:length:'](1, (x['count']()['-'](1)))['join:'](" ")))['eval']()
    }));
    exps = x['fmap:']((function(exp) {
        x = _eval['applyTo:and:'](exp, (this.env));
        return x
    }));
    proc = exps['shift']();
    return proc['message:arguments:']((proc['appropriateApplicationMessage']()), exps)
})
parse = (function(s) {
    return read_from['applyTo:']((tokenize['applyTo:'](s)))
})
tokenize = (function(s) {
    return s['replace:with:']("(", " ( ")['replace:with:'](")", " ) ")['split:'](" ")['filter:']((function(i, v) {
        return v['trim']()['!=:']("")
    }))
})
read_from = (function(tokens) {
    tokens['count']()['ifFalse:']((function() {
        thisBlock['error:']("unexpected EOF while reading")
    }));
    token = tokens['shift']();
    '(' ['='](token)['ifTrue:']((function() {
        l = Array['new']();
        (function() {
            return tokens['@'](0)['!=:'](")")
        })['whileTrue:']((function() {
            l['push:']((read_from['applyTo:'](tokens)))
        }));
        tokens['shift']();
        return l
    }))['else:']((function() {
        ((token['='](")")))['ifTrue:']((function() {
            thisBlock['error:']("unexpected )")
        }));
        return atom['applyTo:'](token)
    }))
})
atom = (function(token) {
    number = token['toNumber']();
    ((number['toString']()['='](token)))['ifTrue:']((function() {
        return number
    }));
    return token['toString']()
})
to_string = (function(exp) {
    ((Reflect['isObject:linkedTo:'](exp, Array)))['ifTrue:']((function() {
        return '(' ['+']((exp['fmap:'](to_string)['join:'](" ")))['+'](")")
    }));
    return exp['toString']()
})
repl = (function(str) {
    result = _eval['applyTo:and:']((parse['applyTo:'](str)), GlobalEnv);
    return result['isNil']()['either:or:'](NULL, (function() {
        return to_string['applyTo:'](result)
    }))
})
get_input = (function(inner, before) {
        Pen['cyan']()['write:'](inner)['resetColor']();
        inp = Program['waitForInput']()['trim']();
        ((ss = ((inp['countOf:']("(")))['-']((inp['countOf:'](")")))['+'](before)))['!=:'](0)['ifTrue:']((function() {
            return inp['+'](" ")['+']((get_input['applyTo:and:']((('...' ['*']((ss['+'](1)))))['+'](" "), ss)))
        }));
        return inp
    })
    (function() {
        return true
    })['whileTrue:']((function() {
        Pen['write:']((repl['applyTo:']((get_input['applyTo:and:'](">>> ", 0)))))['brk']()
    }))