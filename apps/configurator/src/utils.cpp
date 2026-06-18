#include <utils.h>

namespace Kub3
{

    QString camelToNormal(const QString &s)
    {
        QString out;
        out.reserve(s.size() + 4);
        bool start = true;
        for (QChar qc : s)
        {
            if (qc.isUpper())
            {
                if (!out.isEmpty() && out.back() != QChar(' '))
                    out.append(' ');
                out.append(qc); // keep uppercase (e.g., HTML)
                start = false;
            }
            else if (qc.isDigit())
            {
                out.append(qc);
                start = false;
            }
            else if (qc.isLower())
            {
                if (start)
                {
                    out.append(qc.toUpper());
                    start = false;
                }
                else
                {
                    out.append(qc);
                }
            }
            else
            {
                if (!out.isEmpty() && out.back() != QChar(' '))
                    out.append(' ');
                start = true;
            }
        }
        return out;
    }

    QString camelToNormal(const std::string &s)
    {
        return camelToNormal(QString::fromStdString(s));
    }

}
