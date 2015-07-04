#include <stdio.h>
#include <stdlib.h>
#include <QDir>

/*
 * Takes a directory <DIR> as input and generates the header and source files <DIR>.h and <DIR>.cpp.
 * Each file in this directory will be transformed in an array of unsigned char.
 */

void usage(char *prog){
  printf("USAGE: %s <DIR> [--header_dir=<HDIR>] [--source_dir=<SDIR>]\n", prog);
}

int main(int argc, char **argv)
{
  QDir dir, header_dir, source_dir;
  QString include_guard, file_name;
  QStringList files, data_names;
  QList<int> data_sizes;
  QFile header, source;
  QFile file;
  QByteArray data;
  int i, num_files, max_name_size;

  if(argc > 4){
    usage(argv[0]);
    return 1;
  }
  dir=QDir(argv[1]);
  if(!dir.isReadable()){
    printf("Could not open directory %s.\n", argv[1]);
    return 1;
  }
  header_dir=QDir(".");
  source_dir=QDir(".");
  for(i=2; i<argc; i++){
    QString arg = QString(argv[i]);
    if(arg.startsWith("--header_dir=")){
      header_dir=QDir(arg.mid(arg.indexOf('=')+1));
      if(!header_dir.isReadable()){
        printf("Could not open header directory %s.\n", header_dir.path().toStdString().data());
        return 1;
      }
    }
    else if(arg.startsWith("--source_dir=")){
      source_dir=QDir(arg.mid(arg.indexOf('=')+1));
      if(!source_dir.isReadable()){
        printf("Could not open header directory %s.\n", source_dir.path().toStdString().data());
        return 1;
      }
    }
    else{
      usage(argv[0]);
      return 1;
    }
  }

  // write the source file
  source.setFileName(source_dir.path() + dir.separator() + dir.dirName() + ".cpp");
  source.open(QIODevice::WriteOnly);
  source.write(("#include \"" + dir.dirName() + ".h\"\n").toStdString().data());

  // names
  files = dir.entryList(QDir::Files);
  num_files = files.size();
  max_name_size=0;
  for(i=0; i<num_files; i++)
    if(max_name_size < files[i].length())
      max_name_size = files[i].length();
  source.write(("\nchar " + dir.dirName().toUpper() + "_FILE_NAMES[][%2] = {\n    ").
               arg(max_name_size+1).toStdString().data());
  for(i=0; i<num_files; i++){
    source.write(("\"" + files[i] + "\"").toStdString().data());
    if(i < (num_files-1))
      source.write(",\n    ");
    else
      source.write("\n");
  }
  source.write("};\n");

  // data
  while(!files.isEmpty()){
    file_name = files.takeFirst();
    file.setFileName(dir.absoluteFilePath(file_name));
    data_names.append(file_name.toUpper().replace('.',"_"));
    source.write(("\nunsigned char " + data_names.last() + "[] = {\n    ").toStdString().data());
    file.open(QIODevice::ReadOnly);
    data = file.read(file.size());
    data_sizes.append(data.size());
    for(i=0; i<data.size(); i++){
      source.write((QString("0x%1").arg((unsigned char)(data[i]), 2, 16, QChar('0'))).toStdString().data());
      if(i == (data.size()-1))
        source.write("\n");
      else if((i%16) == 15)
        source.write(",\n    ");
      else
        source.write(", ");
    }
    source.write("};\n");
    file.close();
  }

  // sizes
  source.write(("\nint " + dir.dirName().toUpper() + "_FILE_SIZES[] = {\n    ").toStdString().data());
  for(i=0; i<data_sizes.size(); i++){
    source.write(QString("%1").arg(data_sizes[i]).toStdString().data());
    if(i < (data_sizes.size()-1))
      source.write(",\n    ");
    else
      source.write("\n");
  }
  source.write("};\n");

  // data array
  source.write(
      ("\nunsigned char* " + dir.dirName().toUpper() + "_FILE_DATA[] = {\n    ").toStdString().data());
  for(i=0; i<data_names.size(); i++){
    source.write(data_names[i].toStdString().data());
    if(i < (data_names.size()-1))
      source.write(",\n    ");
    else
      source.write("\n");
  }
  source.write("};\n");
  source.close();


  // write the header file
  header.setFileName(header_dir.path() + dir.separator() + dir.dirName() + ".h");
  header.open(QIODevice::WriteOnly);

  include_guard = "__" + dir.dirName().toUpper() + "_H__";
  header.write(("#ifndef " + include_guard + "\n").toStdString().data());
  header.write(("#define " + include_guard + "\n\n").toStdString().data());

  header.write(("extern char " + dir.dirName().toUpper() + "_FILE_NAMES[%1][%2];\n").
               arg(num_files).arg(max_name_size+1).toStdString().data());
  header.write(("extern int " + dir.dirName().toUpper() + "_FILE_SIZES[%1];\n").
               arg(num_files).toStdString().data());
  header.write(("extern unsigned char* " + dir.dirName().toUpper() + "_FILE_DATA[%1];\n\n").
               arg(num_files).toStdString().data());

  for(i=0; i<num_files; i++)
    header.write(
          ("extern unsigned char " + data_names[i] + "[%1];\n").arg(data_sizes[i]).toStdString().data());
  header.write(("\n#endif //" + include_guard + "\n").toStdString().data());
  header.close();

  return 0;
}
