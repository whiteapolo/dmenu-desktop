import std.stdio;
import std.file;
import std.string;
import std.process;
import std.typecons : tuple, Tuple;
import std.regex;
import core.stdc.stdlib;
import std.exception;
import std.path : expandTilde;

struct DesktopFile {
    string name;
    string exec;
}

string[] desktopFileDirs = [
    "/usr/share/applications",
    "~/.local/share/applications",
    "/var/lib/flatpak/exports/share/applications",
];

DesktopFile proccessDesktopFile(string pathname)
{
    string content = readText(pathname);

    auto execMatch = matchFirst(content, regex(`Exec=([^\n]*)`));
    auto nameMatch = matchFirst(content, regex(`Name=([^\n]*)`));

    enforce(nameMatch && execMatch, "name and exec was not found");

    auto name = nameMatch.captures[1].strip();
    auto exec = execMatch.captures[1].replaceAll(regex(`%\w+`), "").strip();

    return DesktopFile(name, exec);
}

void proccessDir(string pathname, ref string[string] programs)
{
    foreach (de; dirEntries(expandTilde(pathname), SpanMode.shallow)) {
        if (de.isFile && de.name.endsWith(".desktop")) {
            try {
                DesktopFile file = proccessDesktopFile(de.name);
                programs[file.name] = file.exec;
            } catch (Exception e) {}
        }
    }
}

string[string] proccessDirs(string[] dirs)
{
    string[string] programs;

    foreach (dir; desktopFileDirs) {
        proccessDir(dir, programs);
    }

    return programs;
}

int main(string[] args)
{
    auto pipe = pipeProcess(["dmenu"] ~ args[1 .. $], Redirect.stdin | Redirect.stdout);
    auto programs = proccessDirs(desktopFileDirs);

    foreach (program; programs.keys) {
        pipe.stdin.writeln(program);
    }

    pipe.stdin.close();
    string selectedProgram = pipe.stdout.readln().strip();
    pipe.stdout.close();

    if (selectedProgram.empty) {
        return 1;
    }

    writeln("Selected program: ", selectedProgram);

    if (auto exec = programs.get(selectedProgram, null)) {
        writeln("Running: '", exec, "'");
        system(exec.toStringz());
        return 0;
    } else {
        writeln("program doesn't exitst: ", selectedProgram);
        return 2;
    }
}
