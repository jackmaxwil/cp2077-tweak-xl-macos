// Hot reload: the tweakxl_reload key (\, bound in r6/input/tweakxl.xml) re-reads r6/tweaks and applies the changes
// without restarting the game. Records that existing objects already copied (an equipped item's stats, a spawned
// NPC) take effect when those objects are created again.
module TweakXL

@addField(PlayerPuppet)
private let tweakXLReloadListener: ref<TweakXLReloadListener>;

@wrapMethod(PlayerPuppet)
protected cb func OnGameAttached() -> Bool {
    let result = wrappedMethod();
    if !IsDefined(this.tweakXLReloadListener) {
        this.tweakXLReloadListener = new TweakXLReloadListener();
        this.tweakXLReloadListener.player = this;
        this.RegisterInputListener(this.tweakXLReloadListener, n"tweakxl_reload");
    }
    return result;
}

@wrapMethod(PlayerPuppet)
protected cb func OnDetach() -> Bool {
    let result = wrappedMethod();
    if IsDefined(this.tweakXLReloadListener) {
        this.UnregisterInputListener(this.tweakXLReloadListener);
        this.tweakXLReloadListener = null;
    }
    return result;
}

public class TweakXLReloadListener {
    public let player: wref<PlayerPuppet>;

    protected cb func OnAction(action: ListenerAction, consumer: ListenerActionConsumer) -> Bool {
        if Equals(ListenerAction.GetName(action), n"tweakxl_reload")
            && Equals(ListenerAction.GetType(action), gameinputActionType.BUTTON_RELEASED) && IsDefined(this.player) {
            TweakXL.Reload();
            let message: SimpleScreenMessage;
            message.isShown = true;
            message.duration = 3.0;
            message.message = "TweakXL: tweaks reloaded";
            GameInstance.GetBlackboardSystem(this.player.GetGame()).Get(GetAllBlackboardDefs().UI_Notifications)
                .SetVariant(GetAllBlackboardDefs().UI_Notifications.WarningMessage, ToVariant(message), true);
        }
        return false;
    }
}
