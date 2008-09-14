QT_BEGIN_NAMESPACE

// QStringMatcher.h
class QStringMatcherPrivate;
class Q_CORE_EXPORT QStringMatcher
{
public:
    QStringMatcher();
    QStringMatcher(const QString &pattern,
                   bool cs = true);
    QStringMatcher(const QStringMatcher &other);
    ~QStringMatcher();

    QStringMatcher &operator=(const QStringMatcher &other);

    void setPattern(const QString &pattern);
    void setCaseSensitivity(bool cs);

    int indexIn(const QString &str, int from = 0) const;
    inline QString pattern() const { return q_pattern; }
    inline bool caseSensitivity() const { return q_cs; }

private:
    QStringMatcherPrivate *d_ptr;
    QString q_pattern;
    bool q_cs;
    uint q_skiptable[256];
};

// internal
int qFindString(const ushort *haystack, int haystackLen, int from,
    const ushort *needle, int needleLen, bool cs);
int qFindStringBoyerMoore(const ushort *haystack, int haystackLen, int from,
    const ushort *needle, int needleLen, bool cs);

// QStringMatcher.cpp
static void bm_init_skiptable(const ushort *uc, int l, uint *skiptable, bool cs)
{
    int i = 0;
    register uint *st = skiptable;
    while (i++ < 256 / 8) {
        *st++ = l; *st++ = l; *st++ = l; *st++ = l;
        *st++ = l; *st++ = l; *st++ = l; *st++ = l;
    }
    if (cs) {
        while (l--) {
            skiptable[*uc & 0xff] = l;
            uc++;
        }
    } else {
        const ushort *start = uc;
        while (l--) {
            skiptable[foldCase(uc, start) & 0xff] = l;
            uc++;
        }
    }
}

static inline int bm_find(const ushort *uc, uint l, int index, const ushort *puc, uint pl,
                          const uint *skiptable, bool cs)
{
    if (pl == 0)
        return index > (int)l ? -1 : index;
    const uint pl_minus_one = pl - 1;

    register const ushort *current = uc + index + pl_minus_one;
    const ushort *end = uc + l;
    if (cs) {
        while (current < end) {
            uint skip = skiptable[*current & 0xff];
            if (!skip) {
                // possible match
                while (skip < pl) {
                    if (*(current - skip) != puc[pl_minus_one-skip])
                        break;
                    skip++;
                }
                if (skip > pl_minus_one) // we have a match
                    return (current - uc) - pl_minus_one;

                // in case we don't have a match we are a bit inefficient as we only skip by one
                // when we have the non matching char in the string.
                if (skiptable[*(current - skip) & 0xff] == pl)
                    skip = pl - skip;
                else
                    skip = 1;
            }
            if (current > end - skip)
                break;
            current += skip;
        }
    } else {
        while (current < end) {
            uint skip = skiptable[foldCase(current, uc) & 0xff];
            if (!skip) {
                // possible match
                while (skip < pl) {
                    if (foldCase(current - skip, uc) != foldCase(puc + pl_minus_one - skip, puc))
                        break;
                    skip++;
                }
                if (skip > pl_minus_one) // we have a match
                    return (current - uc) - pl_minus_one;
                // in case we don't have a match we are a bit inefficient as we only skip by one
                // when we have the non matching char in the string.
                if (skiptable[foldCase(current - skip, uc) & 0xff] == pl)
                    skip = pl - skip;
                else
                    skip = 1;
            }
            if (current > end - skip)
                break;
            current += skip;
        }
    }
    return -1; // not found
}

/*! 
    \class QStringMatcher
    \brief The QStringMatcher class holds a sequence of characters that
    can be quickly matched in a Unicode string.

    \ingroup tools
    \ingroup text

    This class is useful when you have a sequence of \l{QChar}s that
    you want to repeatedly match against some strings (perhaps in a
    loop), or when you want to search for the same sequence of
    characters multiple times in the same string. Using a matcher
    object and indexIn() is faster than matching a plain QString with
    QString::indexOf() if repeated matching takes place. This class
    offers no benefit if you are doing one-off string matches.

    Create the QStringMatcher with the QString you want to search
    for. Then call indexIn() on the QString that you want to search.

    \sa QString, QByteArrayMatcher, QRegExp
*/

/*!
    Constructs an empty string matcher that won't match anything.
    Call setPattern() to give it a pattern to match.
*/
QStringMatcher::QStringMatcher()
    : d_ptr(0), q_cs(true)
{
    qMemSet(q_skiptable, 0, sizeof(q_skiptable));
}

/*!
    Constructs a string matcher that will search for \a pattern, with
    case sensitivity \a cs.

    Call indexIn() to perform a search.
*/
QStringMatcher::QStringMatcher(const QString &pattern, bool cs)
    : d_ptr(0), q_pattern(pattern), q_cs(cs)
{
    bm_init_skiptable((const ushort *)pattern.unicode(), pattern.size(), q_skiptable, cs);
}

/*!
    Copies the \a other string matcher to this string matcher.
*/
QStringMatcher::QStringMatcher(const QStringMatcher &other)
    : d_ptr(0)
{
    operator=(other);
}

/*!
    Destroys the string matcher.
*/
QStringMatcher::~QStringMatcher()
{
}

/*!
    Assigns the \a other string matcher to this string matcher.
*/
QStringMatcher &QStringMatcher::operator=(const QStringMatcher &other)
{
    q_pattern = other.q_pattern;
    q_cs = other.q_cs;
    qMemCopy(q_skiptable, other.q_skiptable, sizeof(q_skiptable));
    return *this;
}

/*!
    Sets the string that this string matcher will search for to \a
    pattern.

    \sa pattern(), setCaseSensitivity(), indexIn()
*/
void QStringMatcher::setPattern(const QString &pattern)
{
    bm_init_skiptable((const ushort *)pattern.unicode(), pattern.size(), q_skiptable, q_cs);
    q_pattern = pattern;
}

/*!
    Sets the case sensitivity setting of this string matcher to \a
    cs.

    \sa caseSensitivity(), setPattern(), indexIn()
*/
void QStringMatcher::setCaseSensitivity(bool cs)
{
    if (cs == q_cs)
        return;
    bm_init_skiptable((const ushort *)q_pattern.unicode(), q_pattern.size(), q_skiptable, cs);
    q_cs = cs;
}

/*!
    Searches the string \a str from character position \a from
    (default 0, i.e. from the first character), for the string
    pattern() that was set in the constructor or in the most recent
    call to setPattern(). Returns the position where the pattern()
    matched in \a str, or -1 if no match was found.

    \sa setPattern(), setCaseSensitivity()
*/
int QStringMatcher::indexIn(const QString &str, int from) const
{
    if (from < 0)
        from = 0;
    return bm_find((const ushort *)str.unicode(), str.size(), from,
                   (const ushort *)q_pattern.unicode(), q_pattern.size(),
                   q_skiptable, q_cs);
}

/*!
    \fn QString QStringMatcher::pattern() const

    Returns the string pattern that this string matcher will search
    for.

    \sa setPattern()
*/

/*!
    \fn Qt::CaseSensitivity QStringMatcher::caseSensitivity() const

    Returns the case sensitivity setting for this string matcher.

    \sa setCaseSensitivity()
*/

/*!
    \internal
*/

int qFindStringBoyerMoore(
    const ushort *haystack, int haystackLen, int haystackOffset,
    const ushort *needle, int needleLen, bool cs)
{
    uint skiptable[256];
    bm_init_skiptable((const ushort *)needle, needleLen, skiptable, cs);
    if (haystackOffset < 0)
        haystackOffset = 0;
    return bm_find((const ushort *)haystack, haystackLen, haystackOffset,
                   (const ushort *)needle, needleLen, skiptable, cs);
}

QT_END_NAMESPACE
