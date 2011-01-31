#include "foulstate.h"
#include "game.h"
#include "goalscoredstate.h"
#include "pitch.h"
#include "ball.h"
#include "Player.h"
#include "screengraphics.h"

FoulState::FoulState(Game *g, Pitch *p)
    : QState(g),
      m_game(g),
      m_pitch(p)
{
    m_takePositions = new QState(this);
    m_allDone = new QFinalState(this);
    setInitialState(m_takePositions);

    m_timeLineTakePositions = new QTimeLine(1000*3, this);
    m_timeLineTakePositions->setCurveShape(QTimeLine::LinearCurve);
    m_timeLineTakePositions->setFrameRange(0, 100);

    m_takePositions->addTransition(m_takePositions, SIGNAL(finished()), m_allDone);

    connect(m_timeLineTakePositions, SIGNAL(frameChanged(int)), this, SLOT(playFrame(int)));
    connect(m_timeLineTakePositions, SIGNAL(finished()), m_game, SLOT(kickOff()));
}

void FoulState::onEntry(QEvent * /* event */)
{
    qDebug() << "FoulState::onEntry";
    m_game->stopGameClock();
    createPlayerAnimationItems(TakePositions);
    m_timeLineTakePositions->start();
}

// animate from present player position to another point.
void FoulState::createPlayerAnimationItems(GameState g)
{
    m_playerAnimationItems.clear(); // TODO XXX TIM delete all

    foreach (Player *p, m_pitch->m_players) {
        QGraphicsItemAnimation* anim = new QGraphicsItemAnimation(this);
        anim->setItem(p);
        m_playerAnimationItems.append(anim);

        QPointF tmp;
        qreal stepX;
        qreal stepY;

        switch( g )
        {
        case TakePositions:
        {
            anim->setTimeLine(m_timeLineTakePositions);
            tmp = p->pos();
            stepX = ( m_pitch->ball()->pos().x() - tmp.x()) / 100.0;
            stepY = ( m_pitch->ball()->pos().y() - tmp.y()) / 100.0;
            MWindow::Action a = calculateAction(tmp, p->m_startPositionRectF.center());
            p->movePlayer(a);
        }
            break;

        default:
            break;
        }

        for (int i = 0; i < 100; ++i) {
            anim->setPosAt(i / 100.0, QPointF(tmp.x() + stepX,
                                              tmp.y() + stepY));
            tmp.setX(tmp.x() + stepX);
            tmp.setY(tmp.y() + stepY);
        }
    }
}

void FoulState::playFrame(int frame)
{
    qreal f = frame/ 100.00;
    foreach (QGraphicsItemAnimation *anim, m_playerAnimationItems)
        anim->item()->setPos(anim->posAt(f));
    m_pitch->m_scene->update();
}