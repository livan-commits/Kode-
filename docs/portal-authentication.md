# Portail — authentification et récupération de compte

## Implémenté

Le portail `https://kode-ecosystem.vercel.app` utilise le client public Supabase JS et le projet Kode- dédié.

- Inscription par e-mail et mot de passe.
- Connexion par mot de passe.
- Déconnexion et restauration de session.
- Demande de réinitialisation de mot de passe.
- Écran de définition du nouveau mot de passe lors de l'événement `PASSWORD_RECOVERY`.
- Tableau de bord chargé avec le JWT utilisateur via `account-dashboard`.
- Aucun secret de service ni jeton fournisseur n'est embarqué dans le navigateur.

## Configuration Supabase requise

Dans Supabase Dashboard → Authentication → URL Configuration :

- Site URL : `https://kode-ecosystem.vercel.app`
- Ajouter aux Redirect URLs : `https://kode-ecosystem.vercel.app/**`

L'URL de redirection doit correspondre exactement à l'origine du portail. La configuration du projet n'est pas modifiée automatiquement par le code du dépôt ; elle doit être vérifiée dans les paramètres Auth du projet.

## Modèle de sécurité

- La clé `sb_publishable_...` côté navigateur est une clé publique, pas une clé service-role.
- Les données du tableau de bord sont récupérées après validation de l'utilisateur par `auth.getUser()`.
- Les lectures sont exécutées avec le JWT utilisateur et restent soumises à RLS.
- Les tokens et secrets des fournisseurs ne sont jamais renvoyés au navigateur.
- Les erreurs backend sont affichées sans présenter de compte externe comme connecté lorsqu'il ne l'est pas.

## Limites / vérifications restantes

- Les parcours doivent encore être testés avec une adresse e-mail réelle (confirmation, connexion, reset et changement du mot de passe).
- L'envoi des e-mails dépend de la configuration SMTP et des templates Auth du projet.
- Les fournisseurs externes ne sont pas activés par cette authentification : leurs consentements OAuth, clés développeur et tests restent spécifiques à chaque plateforme.
- Aucun provider gaming ou domotique ne doit être marqué disponible avant tests réels de bout en bout.
