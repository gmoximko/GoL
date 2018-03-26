#include <tuple>

#include <QDir>
#include <QResource>
#include <QTextStream>
#include <QRegExp>
#include <QException>

#include "rleparser.h"
#include "qtutilities.h"

namespace Utilities {

namespace {

void readPoints(QTextStream& stream, QPoint size, Logic::Points& points)
{
  int cell_x = 0;
  int cell_y = 0;
  int index = 0;

  QString line;
  while (stream.readLineInto(&line))
  {
    Q_ASSERT(QRegExp("[\\dboxy$!]+").exactMatch(line));

    for (QChar const c : line)
    {
      if (c == '!')
      {
        return;
      }
      else if (c.isDigit())
      {
        index *= 10;
        index += c.digitValue();
      }
      else
      {
        index = (index > 0) ? index : 1;
        if (c == '$')
        {
          cell_x = 0;
          cell_y += index;
        }
        else if (c.toLower() == 'b')
        {
          cell_x += index;
        }
        else /*if (c.toLower()  == 'o')*/
        {
          for (int x = 0; x < index; x++, cell_x++)
          {
            QPoint unit(cell_x, size.y() - cell_y - 1);
            points.push_back(unit - size / 2);
          }
        }
        index = 0;
      }
      Q_ASSERT(cell_x >= 0 && cell_x <= size.x());
      Q_ASSERT(cell_y >= 0 && cell_y <= size.y());
    }
  }
  Q_ASSERT(false);
}

std::tuple<QString, QPoint, Logic::Points> readPattern(QTextStream& stream)
{
  QRegExp rname("#[Nn] (.*)");
  QRegExp rsize("x *= *(\\d+).+y *= *(\\d+).*");

  QString name;
  QPoint size;
  Logic::Points points;

  QString line;
  while (stream.readLineInto(&line))
  {
    if (rname.exactMatch(line))
    {
      Q_ASSERT(rname.captureCount() > 0);
      name = rname.cap(1);
    }
    else if (rsize.exactMatch(line))
    {
      Q_ASSERT(rsize.captureCount() > 1);
      size = QPoint(rsize.cap(1).toInt(), rsize.cap(2).toInt());
      break;
    }
  }
  readPoints(stream, size, points);
  return std::make_tuple(std::move(name), size, std::move(points));
}

class PatternImpl : public Logic::Pattern
{
public:
  explicit PatternImpl(QResource const& resource)
  {
    QByteArray bytes;
    auto const byte_count = static_cast<int>(resource.size());
    if (resource.isCompressed())
    {
      bytes = qUncompress(resource.data(), byte_count);
    }
    else
    {
      bytes = QByteArray(reinterpret_cast<char const*>(resource.data()), byte_count);
    }
    Q_ASSERT(!bytes.isEmpty());

    QTextStream stream(bytes);
    std::tie(name_, size_, points_) = readPattern(stream);

    Q_ASSERT(!name_.isEmpty());
    Q_ASSERT(size_ != QPoint());
    Q_ASSERT(!points_.empty());
  }

  QString name() const override
  {
    return name_;
  }
  Logic::Points const& points() const override
  {
    return points_;
  }
  QPoint size() const override
  {
    return size_;
  }
  Logic::SizeT scores() const override
  {
    return points_.size();
  }

private:
  QString name_;
  QPoint size_;
  Logic::Points points_;
};

class PatternsImpl : public Logic::Patterns
{
public:
  explicit PatternsImpl(QString patterns_path)
    : patterns_(patterns_path)
  {
    Q_ASSERT(!patterns_.isEmpty(QDir::Files));
  }

  Logic::SizeT patternCount() const override
  {
    return patterns_.count();
  }
  Logic::PatternPtr patternAt(Logic::SizeT idx) const override
  {
    Q_ASSERT(idx >= 0 && idx < patternCount());
    auto const all_patterns = patterns_.entryInfoList(QDir::Files);
    auto const& file_info = all_patterns.at(idx);
    QResource const resource(file_info.filePath());
    Q_ASSERT(resource.isValid());
    return Qt::makeShared<PatternImpl>(resource);
  }

private:
  QDir const patterns_;
};

} // namespace

Logic::PatternsPtr createPatterns()
{
  return Qt::makeShared<PatternsImpl>(":/Patterns/Patterns");
}

} // Utilities
