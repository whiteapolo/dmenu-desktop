import std.datetime : SysTime;
import std.stdio : writeln, writefln;
import std.file : getTimes, rename, remove, copy;
import core.stdc.stdlib : exit;
import std.process : wait, spawnProcess;
import std.algorithm : any, map;
import std.getopt;

string TARGET = "dmenu-desktop";
string[] SRC = ["main.d"];
string INSTALL_FOLDER = "/usr/local/bin";

SysTime getModificationTime(string fileName)
{
  SysTime accessTime;
  SysTime modificationTime;
  getTimes(fileName, accessTime, modificationTime);
  return modificationTime;
}

bool should_rebuild(string binary, string[] sources)
{
  try {
    SysTime binaryModTime = getModificationTime(binary);
    auto srcModTime = sources.map!(src => getModificationTime(src));

    return any!(a => a > binaryModTime)(srcModTime);
  }
  catch (Exception e) {
    return true;
  }
}

int run(string[] args)
{
  writeln("[CMD]: ", args);
  auto status = wait(spawnProcess(args));

  if (status) {
    writeln("[ERROR]: exited abnormally with code ", status);
  }

  return status;
}

int removeLog(string file)
{
  writeln("[REMOVE]: ", file);

  try {
    remove(file);
  } catch (Exception e) {
    writeln("[ERROR]: ", e.msg);
    return 1;
  }

  return 0;
}

int renameLog(string src, string target)
{
  writefln("[RENAME]: %s -> %s", src, target);

  try {
    rename(src, target);
  } catch (Exception e) {
    writeln("[ERROR]: ", e.msg);
    return 1;
  }

  return 0;
}

int copyLog(string src, string target)
{
  writefln("[COPY]: %s -> %s", src, target);

  try {
    copy(src, target);
  } catch (Exception e) {
    writeln("[ERROR]: ", e.msg);
    return 1;
  }

  return 0;
}

void rebuild_youself(string binary)
{
    if (!should_rebuild(binary, [__FILE__])) {
        return;
    }

    renameLog(binary, binary ~ ".old");
    auto status = run(["rdmd", "--build-only", __FILE__, "-of=" ~ binary]);

    if (status != 0) {
        writeln("[ERROR]: compilation failed");
        renameLog(binary ~ ".old", binary);
        exit(status);
    }

    removeLog(binary ~ ".old");

    run(["./" ~ binary]);
    exit(0);
}

int build()
{
  if (!should_rebuild(TARGET, SRC)) {
    return 0;
  }

  return run(["rdmd", "--build-only", "-of=" ~ TARGET] ~ SRC);
}

int clean()
{
  try {
    remove(TARGET);
  } catch (Exception e) {}

  return 0;
}

int install()
{
  int status = build();

  if (status != 0) {
    return status;
  }

  return run(["cp", "-p", TARGET, INSTALL_FOLDER ~ "/" ~ TARGET]);
}

int uninstall()
{
  try {
    removeLog(INSTALL_FOLDER ~ "/" ~ TARGET);
  } catch (Exception e) {
    writeln(e.msg);
    return 1;
  }

  return 0;
}

int main(string[] args)
{
  rebuild_youself("build");

  if (args.length == 1) {
    return build();
  }

  foreach (arg; args[1..$]) {
    switch (arg) {
      case "install": install(); break;
      case "clean": clean(); break;
      case "build": build(); break;
      case "uninstall": uninstall(); break;
      default: writefln("Unknown option '%s'", arg);
    }
  }

  return 0;
}
