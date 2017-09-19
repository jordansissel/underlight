package com.semicomplete.intellij;

import com.intellij.openapi.compiler.CompilationStatusListener;
import com.intellij.openapi.compiler.CompileContext;
import com.intellij.openapi.compiler.CompilerTopics;
import com.intellij.openapi.components.AbstractProjectComponent;
import com.intellij.openapi.project.Project;
import com.intellij.util.messages.MessageBusConnection;
import org.jetbrains.annotations.NotNull;

public class PluginMain extends AbstractProjectComponent {
    private MessageBusConnection bus;
    protected PluginMain(Project project) {
        super(project);
        bus = project.getMessageBus().connect();
    }

    @NotNull
    @Override
    public String getComponentName() {
        return "led-color-status";
    }

    @Override
    public void projectOpened() {
    }

    @Override
    public void projectClosed() {

    }

    @Override
    public void initComponent() {
        bus.subscribe(CompilerTopics.COMPILATION_STATUS, new CompilationStatusHandler());
    }

    @Override
    public void disposeComponent() {
        bus.disconnect();
        //ExecutionManager.getInstance(myProject).getContentManager().removeRunContentListener(this);
    }

    private class CompilationStatusHandler implements CompilationStatusListener {
        @Override
        public void compilationFinished(boolean aborted, int errors, int warnings, CompileContext compileContext) {
            if (!aborted) {
                Display.reportCompilation(errors, warnings);
            }
        }

        @Override
        public void automakeCompilationFinished(int errors, int warnings, CompileContext compileContext) {
        }

        @Override
        public void fileGenerated(String outputRoot, String relativePath) {
        }

    }
}
