namespace eval ansicolor {
    namespace export +
    variable map {
        bold 1 light 2 blink 5 invert 7
        black 30 red 31 green 32 yellow 33 blue 34 purple 35 cyan 36 white 37
        Black 40 Red 41 Green 42 Yellow 43 Blue 44 Purple 45 Cyan 46 White 47
    }

    proc + {args} {
        variable map
        set t 0
        foreach i $args {
            set ix [lsearch -exact $map $i]
            if {$ix>-1} {lappend t [lindex $map [incr ix]]}
        }
        return "\033\[[join $t {;}]m"
    }

    proc get {code} {
        variable map
        set res [list]
        foreach i [split $code ";"] {
            set ix [lsearch -exact $map $i]
            if {$ix>-1} {lappend res [lindex $map [incr ix -1]]}
        }
        set res
    }

    proc text {w args} {
        variable $w.tags ""
        eval ::text $w $args
        rename ::$w ::_$w
        proc ::$w {cmd args} {
            regsub -all @ {([^@]*)(@\[([^m]+)m)} \x1b re
            set self [lindex [info level 0] 0]
            if {$cmd=="insert"} {
                foreach {pos text tags} $args break
                while {[regexp $re $text -> before esc code]} {
                    _$self insert $pos $before [set ansicolor::$self.tags]
                    set pos [_$self index insert]
                    set ansicolor::$self.tags [ansicolor::get $code]
                    regsub $re $text "" text
                }
                _$self insert $pos $text [concat $self.tags $tags]
            } else {
                uplevel 1 _$self $cmd $args
            }
        }
        foreach i {black red green yellow blue purple cyan white} {
            _$w tag configure $i -foreground $i
        }
        foreach i {Black Red Green Yellow Blue Purple Cyan White} {
            _$w tag configure $i -background [string tolower $i]
        }
        set w
    }

    #Test code:

    proc testtext {w args} {
        eval text $w -font {{Courier 9}} $args
        $w insert end "This line comes in blue directly, not via ANSI\n" \
            {blue Yellow}
        $w insert end "Hello [+ red Yellow]ANSI world[+] out there!\n\
            This line has no special markup.\n\
            [+ Black white]Inverted [+] right at the beginning..\
            ... and ending in [+ Blue yellow]Blue yellow [+]\n\
            [test]\n\
            [+ blue]End of test...[+]"
        set w
    }

    proc test {} {
        set res "Hello[+ bold Red white] world [+]again:\n"
        foreach i {black red green yellow blue purple cyan white} {
            append res "[+ bold $i][format %7s $i]: "
            foreach j {Black Red Green Yellow Blue Purple Cyan White} {
                append res "[+ $i $j][format %7s $j]"
            }
            append res \n
        }
        foreach i {bold light blink invert} {
            append res "testing [+ $i]$i mode[+]\n"
        }
        set res
    }

    proc cgrepf {what fp} {
        while {[gets $fp line]>-1} {
            if [regsub -all $what $line "[+ bold red]&[+]" line] {
                puts $line
            }
        }
    }

    proc cgrep {argc argv} {
        if $argc==0 {
            puts [test]
        } elseif $argc==1 {
            cgrepf [lindex $argv 0] stdin
        } else {
            foreach i [lrange $argv 1 end] {
                set fp [open $i]
                cgrepf [lindex $argv 0] $fp
                close $fp
            }
        }
    }
}
