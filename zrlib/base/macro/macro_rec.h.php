<?php
$CONFIG =
[
"nargs.max" => 128,
"num.max" => 1024
];
?>
#ifndef ZRMACRO_REC_H
#define ZRMACRO_REC_H

#define ZRINC(x) ZRCONCAT_2(ZRINC_, x)
<?php
for($i = 0; $i < $CONFIG['num.max']; $i++)echo "#define ZRINC_$i ",$i + 1,"\n";
?>

#define ZRDEC(x) ZRCONCAT_2(ZRDEC_, x)
#define ZRDEC_0 0
<?php
for($i = 1; $i <= $CONFIG['num.max']; $i++)echo "#define ZRDEC_$i ",$i - 1,"\n";
?>

#define ZRNARGS(...) ZRNARGS_(__VA_ARGS__, ZRREVERSE_NARGS_())
#define ZRNARGS_(...) ZRNARGS__(__VA_ARGS__)
#define ZRNARGS__(<?php
for($i = 0; $i < $CONFIG['nargs.max']; $i++)echo "_$i,"
?>N,_...) N
#define ZRREVERSE_NARGS_() <?php
for($i = $CONFIG['nargs.max']; $i >= 0; $i--)echo "$i", $i != 0 ? "," : "";
?>

#define ZRNARGS_MAX <?=$CONFIG['nargs.max']?>

#define ZREMPTY()
#define ZRFORGET(...)
#define ZREXPAND(...) __VA_ARGS__

#define ZRDEFERN(N) ZRCONCAT(ZRDEFER,N)()
<?php
for($i = 0; $i <= $CONFIG['nargs.max']; $i++)
{
	echo "#define ZRDEFER$i() ";

	for($j = 0; $j < $i; $j++)echo "ZREMPTY ";
	for($j = 0; $j < $i; $j++)echo "()";
	echo "\n";
}?>

/* https://gustedt.wordpress.com/2010/06/08/detect-empty-macro-arguments/ */
#define ZRARGS_HAS_COMMA(...) ZRNARGS__(__VA_ARGS__, <?= implode(',', array_fill(0, $CONFIG['nargs.max'] - 1, 1))?> , 0)
#define _TRIGGER_PARENTHESIS_(...) ,
#define _IS_EMPTY_CASE_0001 ,
#define ZRARGS_EMPTY(...) ZRARGS_HAS_COMMA(ZRCONCAT_2(_IS_EMPTY_CASE_, _ZRARGS_EMPTY_BITS(__VA_ARGS__)))
#define ZRARGS_NEMPTY(...) ZRCOMPL(ZRARGS_EMPTY(__VA_ARGS__))

/* test if there is just one argument, eventually an empty one */
/* test if _TRIGGER_PARENTHESIS_ together with the argument
 /* test if the argument together with a adds a comma */
/* test if placing it between _TRIGGER_PARENTHESIS_ and the parenthesis adds a comma */
#define _ZRARGS_EMPTY_BITS(...) ZRCONCAT_4(\
	ZRARGS_HAS_COMMA(__VA_ARGS__), \
	ZRARGS_HAS_COMMA(_TRIGGER_PARENTHESIS_ __VA_ARGS__), \
	ZRARGS_HAS_COMMA(__VA_ARGS__ (/*empty*/)), \
	ZRARGS_HAS_COMMA(_TRIGGER_PARENTHESIS_ __VA_ARGS__ (/*empty*/)))

/* https://github.com/pfultz2/Cloak/wiki/C-Preprocessor-tricks,-tips,-and-idioms */
#define ZREVAL(...)   ZREVAL64(__VA_ARGS__)
#define ZREVAL64(...) ZREVAL32(ZREVAL32(__VA_ARGS__))
#define ZREVAL32(...) ZREVAL16(ZREVAL16(__VA_ARGS__))
#define ZREVAL16(...) ZREVAL8(ZREVAL8(__VA_ARGS__))
#define ZREVAL8(...)  ZREVAL4(ZREVAL4(__VA_ARGS__))
#define ZREVAL4(...)  ZREVAL2(ZREVAL2(__VA_ARGS__))
#define ZREVAL2(...)  ZREVAL1(ZREVAL1(__VA_ARGS__))
#define ZREVAL1(...) __VA_ARGS__

#define ZRCHECK_N(x, n, ...) n
#define ZRCHECK(...) ZRCHECK_N(__VA_ARGS__, 0)
#define ZRPROBE(x) x, 1

#define ZRNOT(x) ZRCHECK(ZRCONCAT_2(ZRNOT_, x)())
#define ZRNOT_0() ZRPROBE(~)
#define ZRNOT_() ZRPROBE(~)

#define ZRCOMPL(b) ZRCONCAT_2(ZRCOMPL_, b)
#define ZRCOMPL_0 1
#define ZRCOMPL_1 0

#define ZRGT_1(N) ZRCOMPL(ZRCHECK(ZRCONCAT(ZRGT_1_, N)()))
#define ZRGT_1_() ZRPROBE(~)
#define ZRGT_1_0() ZRPROBE(~)
#define ZRGT_1_1() ZRPROBE(~)

#define ZRBOOL(x) ZRCOMPL(ZRNOT(x))

#define ZRBITOR(x) ZRCONCAT_2(ZRBITOR, x)
#define ZRBITOR_0(y) y
#define ZRBITOR_1(y) 1

#define ZRBITAND(x) ZRCONCAT_2(ZRBITAND_, x)
#define ZRBITAND_0(y) 0
#define ZRBITAND_1(y) y

#define ZRAND(...) ZRCONCAT(ZRAND_,ZRNARGS(__VA_ARGS__))(__VA_ARGS__)
#define ZRAND_2(_1,_2) ZRCONCAT(ZRBITAND_,ZRBOOL(_1))(ZRBOOL(_2))

#define ZROR(...) ZRCONCAT(ZROR_,ZRNARGS(__VA_ARGS__))(__VA_ARGS__)
#define ZROR_2(_1,_2) ZRCONCAT(ZRBITOR_,ZRBOOL(_1))(ZRBOOL(_2))

#define ZRIIF(c) ZRCONCAT_2(ZRIIF_, c)
#define ZRIIF_0(t, ...) __VA_ARGS__
#define ZRIIF_1(t, ...) t

#define ZRIF_EXPAND(...) __VA_ARGS__
#define ZRIF_FORGET(...)

#define ZRIF(c) ZRIIF(ZRBOOL(c))
#define ZRIFTHENELSE(c,t,...) ZRIF(c) (t,__VA_ARGS__)
#define ZRWHEN(c) ZRIF(c)(ZRIF_EXPAND, ZRIF_FORGET)

#define ZRWHENELSE_WHEN(...) __VA_ARGS__ ZRFORGET
#define ZRWHENELSE_ELSE(...) ZRFORGET (__VA_ARGS__) ZREXPAND
#define ZRWHENELSE(c) ZRIF(c)(ZRWHENELSE_WHEN, ZRWHENELSE_ELSE)

#define ZRREPEAT(count, macro, ...) \
    ZRWHEN(count) \
    ( \
        ZRREPEAT_INDIRECT ZRDEFER2() () \
        ( \
            ZRDEC(count), macro, __VA_ARGS__ \
        ) \
        macro ZRDEFER2() \
        ( \
            ZRDEC(count), __VA_ARGS__ \
        ) \
    )
#define ZRREPEAT_INDIRECT() ZRREPEAT
#define ZRREPEAT_E(...) ZREVAL(ZRREPEAT(__VA_ARGS__))

#define ZRWHILE(pred, op, ...) \
    ZRWHEN(pred(__VA_ARGS__)) \
    ( \
        ZRWHILE_INDIRECT ZRDEFER2() () \
        ( \
            pred, op, op(__VA_ARGS__) \
        ), \
        __VA_ARGS__ \
    )
#define ZRWHILE_INDIRECT() ZRWHILE
#define ZRWHILE_E(...) ZREVAL(ZRWHILE(__VA_ARGS__))

#define ZRARGS_XEVEN_(X,nb,V, ...) \
	X(V) \
	ZRWHEN(ZRGT_1(nb)) \
	(, \
		ZRARGS_XODD_INDIRECT_ ZRDEFER2() () (X,ZRDEC(nb),__VA_ARGS__) \
	)
#define ZRARGS_XODD_(X,nb,V, ...) ZRARGS_XEVEN_(X,ZRDEC(nb),__VA_ARGS__)
#define ZRARGS_XODD_INDIRECT_()  ZRARGS_XODD_

#define ZRARGS_XEVEN(X, ...) ZRARGS_XEVEN_(X,ZRDEC(ZRNARGS(__VA_ARGS__)),__VA_ARGS__)
#define ZRARGS_XODD(X, ...)  ZRARGS_XODD_(X,ZRDEC(ZRNARGS(__VA_ARGS__)),__VA_ARGS__)

#define ZRARGS_EVENODD_EXPAND(...) __VA_ARGS__
#define ZRARGS_EVEN(...) ZRARGS_XEVEN(ZRARGS_EVENODD_EXPAND,__VA_ARGS__)
#define ZRARGS_ODD(...)  ZRARGS_XODD(ZRARGS_EVENODD_EXPAND,__VA_ARGS__)

#define ZRARGS_XEVEN_E(X,...) ZREVAL(ZRARGS_XEVEN(X,__VA_ARGS__))
#define ZRARGS_XODD_E(X,...)  ZREVAL(ZRARGS_XODD(X,__VA_ARGS__))
#define ZRARGS_EVEN_E(...)    ZREVAL(ZRARGS_EVEN(__VA_ARGS__))
#define ZRARGS_ODD_E(...)     ZREVAL(ZRARGS_ODD(__VA_ARGS__))

#define _ZRARGS_XCAPPLY(X, nb_total, depth, rest, V, ...) \
	ZRWHEN(rest) \
	( \
		X(nb_total,depth,V) \
		_ZRARGS_XCAPPLY_INDIRECT ZRDEFER2() () (X,nb_total,ZRINC(depth),ZRDEC(rest),__VA_ARGS__) \
	)
#define _ZRARGS_XCAPPLY_INDIRECT() _ZRARGS_XCAPPLY
#define ZRARGS_XCAPPLY(X,...) _ZRARGS_XCAPPLY(X,ZRNARGS(__VA_ARGS__),0,ZRNARGS(__VA_ARGS__),__VA_ARGS__) \

#define ZRARGS_XCAPPLY_E(X,...) ZREVAL(ZRARGS_XCAPPLY(X,__VA_ARGS__))

#define _ZRARGS_XCAPPLYREC(X, nb_total, depth, rest, A, B, ...) \
	X ZRDEFERN(ZRINC(rest)) (nb_total,depth, A, \
	ZRWHENELSE(rest) \
	( \
		_ZRARGS_XCAPPLYREC_INDIRECT ZRDEFER2() () (X, nb_total, ZRINC(depth), ZRDEC(rest), B, __VA_ARGS__) \
	) \
	(B))
#define _ZRARGS_XCAPPLYREC_INDIRECT() _ZRARGS_XCAPPLYREC
#define ZRARGS_XCAPPLYREC(X,...) _ZRARGS_XCAPPLYREC(X, ZRNARGS(__VA_ARGS__),0,ZRDEC(ZRDEC(ZRNARGS(__VA_ARGS__))),__VA_ARGS__)
#define ZRARGS_XCAPPLYREC_E(X,...) ZREVAL(ZRARGS_XCAPPLYREC(X,__VA_ARGS__))

#endif
