// Copyright 2019 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <QFileOpenEvent>

#include "DolphinQt/QtUtils/FileOpenEventFilter.h"

FileOpenEventFilter::FileOpenEventFilter(QObject* event_source) : QObject(event_source)
{
  event_source->installEventFilter(this);
}

bool FileOpenEventFilter::eventFilter(QObject* object, QEvent* event)
{
  if (event->type() == QEvent::FileOpen)
  {
    auto* openEvent = static_cast<QFileOpenEvent*>(event);
    emit fileOpened(openEvent->file());
    return true;
  }

  return false;
}
