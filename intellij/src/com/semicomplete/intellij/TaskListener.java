package com.semicomplete.intellij;

import com.intellij.openapi.externalSystem.model.task.ExternalSystemTaskId;
import com.intellij.openapi.externalSystem.model.task.ExternalSystemTaskNotificationListenerAdapter;
import org.jetbrains.annotations.NotNull;

public class TaskListener extends ExternalSystemTaskNotificationListenerAdapter {
    @Override
    public void onSuccess(@NotNull ExternalSystemTaskId id) {
        super.onSuccess(id);
    }

    @Override
    public void onFailure(@NotNull ExternalSystemTaskId id, @NotNull Exception e) {
        super.onFailure(id, e);
    }

    @Override
    public void onCancel(@NotNull ExternalSystemTaskId id) {
        super.onCancel(id);
    }
}
