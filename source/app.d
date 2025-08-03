import std.stdio;
import std.file;
import std.string;
import std.process;
import std.typecons : tuple, Tuple;
import std.regex;
import core.stdc.stdlib;
import std.exception;

struct DesktopFile {
  string name;
  string exec;
}

string[] desktopFileDirs = [
  "/usr/share/applications",
  "~/.local/share/applications",
  "/var/lib/flatpak/exports/share/applications",
];

string expandPath(string path)
{
  if (path.startsWith("~")) {
    return environment.get("HOME", "./") ~ path[1 .. $];
  }

  return path;
}


DesktopFile proccessDesktopFile(string pathname)
{
  auto execRegex = ctRegex!"Exec=([^\n]*)";
  auto nameRegex = ctRegex!"Name=([^\n]*)";
  auto fieldCodesRegex = ctRegex!"%/w+";

  string content = readText(pathname);

  auto execMatch = matchFirst(content, execRegex);
  auto nameMatch = matchFirst(content, nameRegex);

  enforce(nameMatch && execMatch, "name and exec was not found");

  auto name = nameMatch.captures[1].strip();
  auto exec = execMatch.captures[1].replaceAll(fieldCodesRegex, "").strip();

  return DesktopFile(name, exec);
}

void proccessDir(string pathname, ref string[string] programs)
{
  foreach (de; dirEntries(expandPath(pathname), SpanMode.shallow)) {
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
  auto programs = proccessDirs(desktopFileDirs);
  return 1;
  auto pipe = pipeProcess(["dmenu"] ~ args[1 .. $], Redirect.stdin | Redirect.stdout);

  scope(exit) {
    pipe.stdin.close();
    pipe.stdout.close();
  }


  foreach (program; programs.keys) {
    pipe.stdin.writeln(program);
  }

  pipe.stdin.close();

  string selectedProgram = pipe.stdout.readln().strip();

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
