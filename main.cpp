#include <iostream>
//#include <cstdio>

#include <QtCore/QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QCommandLineParser>

void log(const QString &msg)
{
  std::wcout << qUtf8Printable(msg);
  //qWarning("%s", qUtf8Printable(msg));
}

void CopyFile(const QString &dest_dir, const QString &absolute_file_path)
{
  if (!QFileInfo::exists(dest_dir))
  {
    if (!QDir().mkpath(dest_dir))
    {
      log(QString("Failed to create directory %1\n").arg(dest_dir));
      return;
    }
  }
  QFileInfo fi(absolute_file_path);
  QString resulting_file_path = dest_dir + "/" + fi.fileName();
  if (!QFile::copy(absolute_file_path, resulting_file_path))
  {
    log(QString("Failed to copy file from %1 to %2\n").arg(absolute_file_path)
        .arg(resulting_file_path));
  }
}

quint16 getCrc(const QString &file_path)
{
  quint16 result = 0;

  QFile file(file_path);
  if (file.open(QIODevice::ReadOnly))
  {
    qint64 file_size = file.size();
    if(file_size)
    {
      QDataStream in(&file);

      char *buf = new char[file_size];
      int num_bytes_read = in.readRawData(buf, file_size);
      if (num_bytes_read != file_size)
      {
        log(QString("Number of bytes read(%) is not equal to file size(%2)"
                    ": %3\n"));
      }
      result = qChecksum(buf, num_bytes_read);
      delete [] buf;
    }
    else
    {
      log(QString("File size is zero: %1\n").arg(file_path));
    }
  }
  else
  {
    log(QString("Failed to open file for reading: %1\n").arg(file_path));
  }


  return result;
}

void processDirectory(const QString &dir_old,
                      const QString &dir_new,
                      const QString &dir_result,
                      const QStringList &ignoreMasks)
{
  QDir qdir(dir_new, "", QDir::Name | QDir::IgnoreCase,
            QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);

  QFileInfoList files = qdir.entryInfoList();
  for (int i = 0; i < files.count(); ++i)
  {
    //qDebug() << files.at(i).absoluteFilePath();
    QFileInfo fi = files.at(i);

    // check for ignores
    bool ignore_this_file = false;
    for (int k = 0; k < ignoreMasks.size(); ++k)
    {
      QRegExp rx(ignoreMasks.at(k));
      rx.setPatternSyntax(QRegExp::Wildcard);
      if (rx.exactMatch(fi.fileName()))
      {
        ignore_this_file = true;
        break;
      }
    }
    if (ignore_this_file)
      continue;

    if (fi.isFile())
    {
      QDir qdir_old(dir_old);
      if(!qdir_old.exists(fi.fileName()))
      {
        CopyFile(dir_result, fi.absoluteFilePath());
      }
      else  // compare crc, if changed then copy
      {

        quint16 crc_new = getCrc(fi.absoluteFilePath());
        quint16 crc_old = getCrc(qdir_old.absoluteFilePath(fi.fileName()));
        log(QString("Comparing crc %1 = %2\n").arg(crc_old).arg(crc_new));
        if (crc_new == 0 || crc_old == 0)
          return;

        if (crc_new != crc_old)
        {
          CopyFile(dir_result, fi.absoluteFilePath());
        }
      }
    }
    else if (fi.isDir())
    {
      processDirectory(dir_old + "/" + fi.fileName(),
                       dir_new + "/" + fi.fileName(),
                       dir_result + "/" + fi.fileName(),
                       ignoreMasks);
      //*/
    }
  }
}

bool commandLineArgumentsValid(QStringList *positionalArgs,
                               QStringList *ignoreMasks)
{
  bool result = true;

  QCommandLineParser parser;
  parser.setApplicationDescription("Creates patches");
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("old",     QCoreApplication::translate("main", "Path to a directory containing old version of files"));
  parser.addPositionalArgument("new",     QCoreApplication::translate("main", "Path to a directory containing new version of files"));
  parser.addPositionalArgument("result",  QCoreApplication::translate("main", "Path to a directory where resulting patch will be stored"));

  QCommandLineOption ignoreOption(QStringList() << "i" << "ignore",
                                  QCoreApplication::translate("main", "Specifies which file or folder to ignore during comparison. Can be used multiple times. e.g. -i .svn -i *.txt"),
                                  QCoreApplication::translate("main", "mask"));
  parser.addOption(ignoreOption);

  // Process the actual command line arguments given by the user
  parser.process(*qApp);

  *positionalArgs = parser.positionalArguments();
  if (positionalArgs->size() < 3)
  {
    fputs(qPrintable(parser.helpText()), stderr);
    result = false;
  }

  *ignoreMasks = parser.values(ignoreOption);

  return result;
}

int main(int argc, char *argv[])
{
  int app_result = 0;

  QCoreApplication a(argc, argv);
  QCoreApplication::setApplicationName("PatchMaker");
  QCoreApplication::setApplicationVersion("1.0");

  QStringList positionalArgs;
  QStringList ignoreMasks;
  if (commandLineArgumentsValid(&positionalArgs, &ignoreMasks))
  {
    processDirectory(positionalArgs.at(0),
                     positionalArgs.at(1),
                     positionalArgs.at(2),
                     ignoreMasks);
  }
  else
  {
    app_result = 1;
  }

  return app_result;
}
