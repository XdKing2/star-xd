#!/bin/bash
# STAR-XD Heroku One-Click Deployer
# Usage: bash <(curl -s https://raw.githubusercontent.com/XdKing2/star-xd/main/lib/heroku.sh)

set -e

BOLD='\033[1m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${CYAN}"
echo "╔═══════════════════════════════════════╗"
echo "║       STAR-XD Heroku Deployer         ║"
echo "║         by XdKing2             ║"
echo "╚═══════════════════════════════════════╝"
echo -e "${NC}"

# Check dependencies
for cmd in git heroku; do
    if ! command -v $cmd &>/dev/null; then
        echo -e "${RED}❌ '$cmd' not found. Please install it first.${NC}"
        [ "$cmd" = "heroku" ] && echo "Install: https://devcenter.heroku.com/articles/heroku-cli"
        exit 1
    fi
done

# Check login
if ! heroku auth:whoami &>/dev/null; then
    echo -e "${YELLOW}🔑 Please login to Heroku:${NC}"
    heroku login
fi

echo ""
echo -e "${BOLD}📋 Enter your bot details:${NC}"
echo ""

# App name
read -p "$(echo -e ${CYAN}App name (e.g. my-star-xd): ${NC})" APP_NAME
APP_NAME=${APP_NAME:-star-xd-bot}

# Session ID
read -p "$(echo -e ${CYAN}Session ID (star~xxxxx): ${NC})" SESSION_ID
if [ -z "$SESSION_ID" ]; then
    echo -e "${RED}❌ Session ID is required!${NC}"
    exit 1
fi

# Owner number
read -p "$(echo -e ${CYAN}Owner WhatsApp number (e.g. 263701234567): ${NC})" OWNER_NUMBER
OWNER_NUMBER=${OWNER_NUMBER:-263000000000}

# Bot name
read -p "$(echo -e ${CYAN}Bot name (default: STAR-XD): ${NC})" BOT_NAME
BOT_NAME=${BOT_NAME:-STAR-XD}

# MongoDB
read -p "$(echo -e ${CYAN}MongoDB URL (recommended, press Enter to skip): ${NC})" MONGO_URL

# Timezone
read -p "$(echo -e ${CYAN}Timezone (default: Africa/Harare): ${NC})" TIMEZONE
TIMEZONE=${TIMEZONE:-Africa/Harare}

echo ""
echo -e "${YELLOW}🚀 Starting deployment...${NC}"
echo ""

# Clone if not in repo
if [ ! -f "heroku.yml" ]; then
    echo -e "${YELLOW}📦 Cloning STAR-XD repo...${NC}"
    git clone https://github.com/XdKing2/star-xd star-xd-deploy
    cd star-xd-deploy
fi

# Create heroku app
echo -e "${YELLOW}📱 Creating Heroku app: ${APP_NAME}${NC}"
heroku create "$APP_NAME" 2>/dev/null || {
    echo -e "${YELLOW}⚠️ App name taken, trying ${APP_NAME}-$(date +%s | tail -c4)${NC}"
    APP_NAME="${APP_NAME}-$(date +%s | tail -c4)"
    heroku create "$APP_NAME"
}

# Set stack to container
echo -e "${YELLOW}🐳 Setting container stack...${NC}"
heroku stack:set container -a "$APP_NAME"

# Set config vars
echo -e "${YELLOW}⚙️ Setting environment variables...${NC}"
heroku config:set \
    SESSION_ID="$SESSION_ID" \
    OWNER_NUMBER="$OWNER_NUMBER" \
    BOT_NAME="$BOT_NAME" \
    TIMEZONE="$TIMEZONE" \
    COMMAND_MODE="public" \
    -a "$APP_NAME"

[ -n "$MONGO_URL" ] && heroku config:set MONGO_URL="$MONGO_URL" -a "$APP_NAME"

# Add heroku remote if not set
git remote get-url heroku &>/dev/null || heroku git:remote -a "$APP_NAME"

# Push
echo -e "${YELLOW}📤 Deploying to Heroku (this may take 3-5 minutes)...${NC}"
git push heroku main

# Scale
echo -e "${YELLOW}⚡ Starting web dyno...${NC}"
heroku ps:scale web=1 -a "$APP_NAME"

echo ""
echo -e "${GREEN}╔═══════════════════════════════════════╗${NC}"
echo -e "${GREEN}║        ✅ Deployment Complete!        ║${NC}"
echo -e "${GREEN}╚═══════════════════════════════════════╝${NC}"
echo ""
echo -e "${BOLD}App:${NC} https://${APP_NAME}.herokuapp.com"
echo -e "${BOLD}Logs:${NC} heroku logs --tail -a ${APP_NAME}"
echo -e "${BOLD}Stop:${NC} heroku ps:scale web=0 -a ${APP_NAME}"
echo ""
echo -e "${CYAN}📱 Scan QR or use Session ID to connect your WhatsApp!${NC}"
