#ifndef SHOPTEMPLATE_H
#define SHOPTEMPLATE_H

#include <QString>
#include <QVariant>
#include <external/mustache.h>
#include <QDebug>


using namespace Mustache;

QString escapeHtml(const QString& input);
QString unescapeHtml(const QString& escaped);

class ShopTemplate : public Renderer {
public:
    ShopTemplate()
        : Renderer()
        , _maxLength(0) {
    }

    void setMaxLength(int len) {
        _maxLength = len;
    }

    void resetPages() {
        _pages.clear();
    }

    QStringList getPages() {
        return _pages;
    }

    QString render(const QString& _template, Context* context)
    {
        m_error.clear();
        m_errorPos = -1;
        m_errorPartial.clear();

        m_tagStartMarker = m_defaultTagStartMarker;
        m_tagEndMarker = m_defaultTagEndMarker;

        return render(_template, 0, _template.length(), context);
    }
private:
    QStringList _pages;
    int _maxLength;

protected:
    QString render(const QString& _template, int startPos, int endPos, Context* context) {
        QString output;
        int lastTagEnd = startPos;

        while (m_errorPos == -1) {
            if (output.length() > _maxLength) {
                // Uh oh, we went over...
                _pages << output;
                output.clear();
            }

            Tag tag = findTag(_template, lastTagEnd, endPos);
            if (tag.type == Tag::Null) {
                output += _template.midRef(lastTagEnd, endPos - lastTagEnd);
                break;
            }
            output += _template.midRef(lastTagEnd, tag.start - lastTagEnd);
            switch (tag.type) {
            case Tag::Value:
            {
                QString value = context->stringValue(tag.key);
                if (tag.escapeMode == Tag::Escape) {
                    value = escapeHtml(value);
                } else if (tag.escapeMode == Tag::Unescape) {
                    value = unescapeHtml(value);
                }
                output += value;
                lastTagEnd = tag.end;
            }
            break;
            case Tag::SectionStart:
            {
                Tag endTag = findEndTag(_template, tag, endPos);
                if (endTag.type == Tag::Null) {
                    if (m_errorPos == -1) {
                        setError("No matching end tag found for section", tag.start);
                    }
                } else {
                    int listCount = context->listCount(tag.key);
                    if (listCount > 0) {
                        for (int i=0; i < listCount; i++) {
                            context->push(tag.key, i);
                            output += render(_template, tag.end, endTag.start, context);
                            context->pop();
                        }
                    } else if (context->canEval(tag.key)) {
                        output += context->eval(tag.key, _template.mid(tag.end, endTag.start - tag.end), this);
                    } else if (!context->isFalse(tag.key)) {
                        context->push(tag.key);
                        output += render(_template, tag.end, endTag.start, context);
                        context->pop();
                    }
                    lastTagEnd = endTag.end;
                }
            }
            break;
            case Tag::InvertedSectionStart:
            {
                Tag endTag = findEndTag(_template, tag, endPos);
                if (endTag.type == Tag::Null) {
                    if (m_errorPos == -1) {
                        setError("No matching end tag found for inverted section", tag.start);
                    }
                } else {
                    if (context->isFalse(tag.key)) {
                        output += render(_template, tag.end, endTag.start, context);
                    }
                    lastTagEnd = endTag.end;
                }
            }
            break;
            case Tag::SectionEnd:
                setError("Unexpected end tag", tag.start);
                lastTagEnd = tag.end;
                break;
            case Tag::Partial:
            {
                QString tagStartMarker = m_tagStartMarker;
                QString tagEndMarker = m_tagEndMarker;

                m_tagStartMarker = m_defaultTagStartMarker;
                m_tagEndMarker = m_defaultTagEndMarker;

                m_partialStack.push(tag.key);

                QString partial = context->partialValue(tag.key);
                output += render(partial, 0, partial.length(), context);
                lastTagEnd = tag.end;

                m_partialStack.pop();

                m_tagStartMarker = tagStartMarker;
                m_tagEndMarker = tagEndMarker;
            }
            break;
            case Tag::SetDelimiter:
                lastTagEnd = tag.end;
                break;
            case Tag::Comment:
                lastTagEnd = tag.end;
                break;
            case Tag::Null:
                break;
            }
        }

        if (!output.isEmpty() && _maxLength > 0) {
            _pages << output;
            output.clear();
        }

        return output;
    }
};

#endif // SHOPTEMPLATE_H
